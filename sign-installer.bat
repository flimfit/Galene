@echo off
echo Signing Installer File
for %%f in (.\Build\*.exe) do "C:\Program Files (x86)\Windows Kits\10\bin\x86\signtool" sign /debug /v /n "Sean Warren" /tr http://timestamp.globalsign.com/?signature=sha2 /td sha256 "%%f"