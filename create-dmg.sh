#!/bin/bash

# Create a nice dmg for app

# first install appdmg e.g.:
# brew install node
# npm install -g appdmg

# sign code (requires that signature is installed in keychain)
security unlock-keychain -p $keychain_password
codesign --verbose --deep -s P6MM899VL9 Build/Release/flim-ui/Galene.app/

GIT_DESCRIBE=$(git describe)
appdmg Assets/appdmg.json "Galene $GIT_DESCRIBE.dmg"