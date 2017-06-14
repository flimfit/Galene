#!/bin/bash

# Create dmg for app
GIT_DESCRIBE=$(git describe)
appdmg Assets/appdmg.json "Galene $GIT_DESCRIBE.dmg"