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

---

## Disclaimer

### Educational and Research Use Only

This software is provided for **educational, security research, and authorized testing purposes only**. It is intended to be used by security professionals, researchers, and students in controlled environments where they have explicit, documented authorization to test the systems involved.

### No Authorization Implied

The existence of this tool does not grant — and should never be interpreted as granting — any authorization to access, inspect, extract data from, or interfere with any computer system, browser profile, account, or network that you do not own or have not been given **written permission** to test.

Accessing another person's account, browser data, session tokens, or credentials without their consent is illegal in most jurisdictions, including but not limited to:

- **United States** — Computer Fraud and Abuse Act (18 U.S.C. § 1030)
- **United Kingdom** — Computer Misuse Act 1990
- **European Union** — Directive 2013/40/EU on attacks against information systems, and national implementations
- **Canada** — Criminal Code § 342.1 (unauthorized use of a computer)
- **Australia** — Criminal Code Act 1995, Part 10.7
- **Bulgaria** — Penal Code, Art. 319a (unauthorized access to computer information)

Violations can carry criminal penalties including imprisonment and fines, plus civil liability. This list is non-exhaustive. You are responsible for knowing and following the laws that apply to you.

### Intended Uses

Permitted and encouraged:

- Security research on systems you own or administer
- Digital forensics on machines in your physical possession
- Academic work in controlled lab environments
- Capture-the-flag competitions
- Authorized red-team engagements under a signed scope of work
- Personal analysis of your own browser profile

### Prohibited Uses

Explicitly prohibited:

- Extracting credentials, cookies, or session tokens from any system you do not own
- Account takeover of any kind
- Accessing another person's data without consent
- Bypassing authentication on any service
- Distributing, selling, or weaponizing this tool against any target
- Using this tool to harass, stalk, surveil, or harm any individual
- Any use that violates the terms of service of any platform this tool interacts with

### Platform Terms of Service

This tool interacts with third-party APIs. Use of this tool may violate the Terms of Service of those platforms (including Roblox Corporation and browser vendors). You are solely responsible for compliance with all applicable ToS.

### No Warranty

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT.

### Limitation of Liability

IN NO EVENT SHALL THE AUTHORS, CONTRIBUTORS, OR MAINTAINERS OF THIS PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

### Reporting Security Issues

If you discover a vulnerability in a platform as a result of using this tool, report it responsibly:

- Roblox — https://hackerone.com/roblox
- Chromium — https://crbug.com/
- Mozilla — https://bugzilla.mozilla.org/

Do not disclose vulnerabilities publicly before the vendor has had a reasonable opportunity to fix them.

### Acceptance

By downloading, cloning, compiling, running, or otherwise using this software, you acknowledge that you have read and understood this disclaimer, that you agree to use the software only in compliance with applicable laws and this document, and that you accept full responsibility for any consequences of your use.

If you do not agree, do not use this software.
