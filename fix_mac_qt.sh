#!/bin/bash
# From: https://github.com/keepassxreboot/keepassxc/wiki/Set-up-Build-Environment-on-OS-X


# Canonical path to qt5 directory
QT5_DIR="/usr/local/Cellar/qt5/$(ls /usr/local/Cellar/qt5 | sort -r | head -n1)"

# Change qt5 framework ids
for framework in $(find "$QT5_DIR/lib" -regex ".*/\(Qt[a-zA-Z]*\)\.framework/Versions/5/\1"); do
    echo "$framework"
    install_name_tool -id "$framework" "$framework"
done