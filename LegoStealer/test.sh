#!/bin/bash
# extract .ROBLOSECURITY from cookies.json and test it against Roblox

if [ ! -f cookies.json ]; then
    echo "[!] cookies.json not found. run: ./cookie_extract.exe > cookies.json"
    exit 1
fi

# pull the cookie value out of the JSON
COOKIE=$(sed -n 's/.*"name":".ROBLOSECURITY","value":"\([^"]*\)".*/\1/p' cookies.json | head -1)

if [ -z "$COOKIE" ]; then
    echo "[!] .ROBLOSECURITY not found in cookies.json"
    exit 1
fi

echo "[+] cookie length: ${#COOKIE}"
echo "[+] prefix: ${COOKIE:0:40}"

echo "[*] querying roblox..."
curl -s -H "Cookie: .ROBLOSECURITY=$COOKIE" \
    https://users.roblox.com/v1/users/authenticated
echo ""