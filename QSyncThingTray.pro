HEADERS       = window.h \
                syncconnector.h
SOURCES       = main.cpp \
                window.cpp \
                syncconnector.cpp
RESOURCES     = \
    qsyncthing.qrc

QT += widgets
QT += network
QT += webkitwidgets

# install
target.path = binary/
INSTALLS += target
CONFIG += c++11
QMAKE_INFO_PLIST = Info.plist
#QMAKE_CXXFLAGS += /wd4996
ICON = Syncthing.icns
