#!/bin/bash
# For OS X

./clean.sh
~/Qt/5.5/clang_64/bin/qmake -spec macx-xcode ../
open QSyncthingTray.xcodeproj
