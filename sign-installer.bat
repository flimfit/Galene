@echo off

REM   This script signs the installer file
REM   Certificate is from Certum (open source)
REM   Using virtual smart card provided by EIDVirtual: https://www.mysmartlogon.com/eidvirtual/
REM   Using SmartCard Tools to automate PIN entry: https://www.mgtek.com/smartcard
REM   Expects PIN to be provided in environment variable %SMARTCARDPIN%
REM   This script will only work in the build PC with the smart card

echo Signing Installer File
for %%f in (.\Build\*.exe) do scsigntool -pin %SMARTCARDPIN% sign /debug /v /n "Sean Warren" /tr http://timestamp.globalsign.com/?signature=sha2 /td sha256 "%%f"