QSyncthingTray
=============
#### A Traybar Application for Syncthing written in C++ 

![Travis CI](https://travis-ci.org/sieren/QSyncthingTray.svg?branch=master "Travis CI
Status")


A cross-platform status bar for [Syncthing](http://syncthing.net/).  
Currently supports **OS X**, **Windows** and **Linux**.

Written in C++ with Qt.

![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingtray.png "Mac OSX ")
![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingtraywin.png "Windows")
![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingubuntu.png "Windows")

## Features

+ Shows number of connections at a glance.
+ Presents Syncthing UI in a separate view instead of using the browser.
+ [Syncthing-inotify](https://github.com/syncthing/syncthing-inotify) included and starts with program launch to keep track of file changes even faster.
+ Supports authenticated HTTPS connections.
+ Uses OS X Notifications about current connection status.

Is there a feature missing? Open an issue, send me an [email](mailto:info@s-r-n.de) or fork this project and add it yourself.


## How To Use It
QSyncthingTray does not come with Syncthing bundled. Therefore it needs to be downloaded from [Syncthing](http://syncthing.net/).
Once you specifiy the path to the 'syncthing' binary it will automatically spawn syncthing when you run QSyncthingTray.

QSyncthingTray is downloadable in the [Releases](https://github.com/sieren/QSyncthingTray/releases) section here on GitHub.

To start Syncthing at boot (OS X):

+ Go to **System Preferences** and **Users & Groups**
+ Drag QSyncthingTray into the **Login Items** list

## Requirements
If you want to use HTTPS to connect to Syncthing on Windows, please download and install the [OpenSSL DLLs](http://slproweb.com/products/Win32OpenSSL.html). Then restart QSyncthingTray.

## Build & Run
+ Get a recent version of Qt (5.5+)  

### Mac & Windows
+ Use either QtCreator or create an XCode or Visual Studio Project with CMake or QMake.  
```
mkdir build && cd build  
cmake ../ -G Xcode
```

### Linux
+ Get the most recent [Qt Version](http://www.qt.io/download/)
+ I advise to use `qmake` for now, as there have been a few minor problems with the cmake script.  
```
cd ./src  
qmake -config release  
make  
./QSyncthingTray
```