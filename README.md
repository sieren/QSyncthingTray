QSyncthingTray
=============
#### A Traybar Application for Syncthing written in C++ 

![Travis CI](https://travis-ci.org/sieren/QSyncthingTray.svg?branch=master "Travis CI
Status") [![Build
status](https://ci.appveyor.com/api/projects/status/6a88vmt8vxpdhcml/branch/master?svg=true)](https://ci.appveyor.com/project/sieren/qsyncthingtray/branch/master)
[![Downloads](https://img.shields.io/github/downloads/sieren/QSyncthingTray/total.svg "Downloads")](https://github.com/sieren/QSyncthingTray/releases)
[![Issues](https://img.shields.io/github/issues/sieren/QSyncthingTray.svg
"Issues")](https://github.com/sieren/QSyncthingTray/issues)



A cross-platform status bar for [Syncthing](http://syncthing.net/).  
Currently supports **OS X**, **Windows** and **Linux**.

Written in C++ with Qt.

## Features

+ Shows number of connections at a glance.
+ Traffic statistics and graphs about throughput and connections.
+ Launches Syncthing and Syncthing-iNotifier if specified.
+ Quickly pause Syncthing with one click.
+ Last Synced Files - Quickly see the recently synchronised files and open their folder.
+ Quick Access to all shared folders.
+ Presents Syncthing UI in a separate view instead of using the browser.
+ Supports authenticated HTTPS connections.
+ Uses System Notifications about current connection status.
+ Toggle for monochrome icon.

Is there a feature missing? Open an issue, send me an [email](mailto:info@s-r-n.de) or fork this project and add it yourself.



## Download

Precompiled binaries for Windows and Mac are downloadable in the [Releases](https://github.com/sieren/QSyncthingTray/releases) section.

## Screenshots

![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingtray.png "Mac OSX ")
![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingubuntu.png "Ubuntu")
![alt text](https://raw.githubusercontent.com/sieren/QSyncthingTray/master/media/qsyncthingtraywin.png "Windows")



## How To Use It
QSyncthingTray does not come with Syncthing bundled. Therefore it needs to be downloaded from [Syncthing](http://syncthing.net/).
Once you specifiy the path to the 'syncthing' binary it will automatically spawn syncthing when you run QSyncthingTray.

To start Syncthing at boot (OS X):

+ Go to **System Preferences** and **Users & Groups**
+ Drag QSyncthingTray into the **Login Items** list

## Requirements
If you want to use HTTPS to connect to Syncthing on Windows, please download and install the [OpenSSL DLLs](http://slproweb.com/products/Win32OpenSSL.html). Then restart QSyncthingTray.

## Build & Run
+ Get a recent version of Qt (5.5+)  
+ QSyncthingTray can be either built with QWebEngine or QtWebView. By default it is built with QWebEngine. To enable QWebView pass `-DQST_BUILD_WEBKIT=1` as an argument to `cmake`.  

### Mac & Windows
+ Use either QtCreator or create an XCode or Visual Studio Project with CMake or QMake.  
```
mkdir build && cd build  
cmake ../ -G Xcode
```

### Linux
+ Get the most recent [Qt Version](http://www.qt.io/download/)
+ Using `cmake`: 
```
export QTDIR=~/Qt/5.5/gcc_64/
mkdir build && cd build
cmake ../ && make
```

+ Using `qmake`: 
```
cd ./src  
./build_linux.sh  
./QSyncthingTray
```
