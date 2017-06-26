#!/bin/bash

# Create a nice dmg for app

# first install appdmg e.g.:
# brew install node
# npm install -g appdmg

GIT_DESCRIBE=$(git describe)
appdmg Assets/appdmg.json "Galene $GIT_DESCRIBE.dmg"