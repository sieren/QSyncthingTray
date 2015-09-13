QSyncthingTray
=============
#### A Traybar Application for Syncthing written in C++ 

![Travis CI](https://travis-ci.org/sieren/QSyncthingTray.svg?branch=master "Travis CI
Status")


A cross-platform status bar for [Syncthing](http://syncthing.net/).  
Currently supports **OS X** and **Windows**.

Written in C++ with Qt.

![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingtray.png "Mac OSX ")
![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingtraywin.png "Mac OSX ")
## How To Use It
QSyncthingTray does not come with Syncthing bundled. Therefore it needs to be downloaded from [Syncthing](http://syncthing.net/).
Once you specifiy the path to the 'syncthing' binary it will automatically spawn syncthing when you run QSyncthingTray.

QSyncthingTray is downloadable in the [Releases](https://github.com/sieren/QSyncthingTray/releases) section here on GitHub.

To start Syncthing at boot:

+ Go to **System Preferences** and **Users & Groups**
+ Drag QSyncthingTray into the **Login Items** list

## Requirements
QSyncthingTray has been targeted for OS X 10.7, however only been tested on 10.10. Feedback is highly appreciated.

## Build & Run
+ Get a recent version of Qt (5.5+)
+ Use either QtCreator, create an XCode Project with the included script (you might need to specify your qmake path) or a regular Makefile
