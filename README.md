# RBX-Session-Toolkit

> Browser cookie inspection and Roblox session analysis toolkit for security research.

![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Python](https://img.shields.io/badge/Python-3.8%2B-blue)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

A research toolkit for inspecting locally-stored browser cookies and validating Roblox session tokens. Built for red-team engagements, security audits, and CTF-style exercises.

---

## Overview

`RBX-Session-Toolkit` provides a set of utilities to:

- **Inspect** Chromium-family (Chrome, Edge, Brave) and Firefox cookie stores on Windows
- **Decrypt** AES-GCM encrypted cookie values using the browser's DPAPI-bound master key
- **Validate** Roblox `.ROBLOSECURITY` session tokens against the public API
- **Enumerate** account-visible data (profile, groups, friends, inventory) for a given session

The toolkit is designed for:

- Security researchers analyzing cookie protection mechanisms
- Red teams auditing session persistence on Windows endpoints
- Forensics analysts inspecting browser artifacts
- CTF players working with browser-cookie challenges

---

## Components

| Module | Language | Purpose |
|---|---|---|
| `core/cookie_extract` | C++17 | Extract + decrypt browser cookies to JSON |
| `api/roblox_api.py` | Python 3 | Validate session token, enumerate account data |
| `loader/` | C# | Optional orchestrator — runs the pipeline end-to-end |
| `test.sh` | Bash | Standalone cookie validation script |

---

## Features

- Supports Chrome, Edge, Brave, and Firefox cookie stores
- Handles Chrome v80+ AES-GCM encryption (DPAPI-wrapped master key)
- Handles Firefox plaintext cookies
- JSON output for easy integration with other tooling
- Stdlib-only Python (no pip dependencies)
- Static C++ binary — no runtime deps

---

## Build

### Prerequisites (Windows)

Install MSYS2 from https://www.msys2.org. Open the **UCRT64** (or CLANG64) shell.

```bash
pacman -S --needed \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-sqlite3
