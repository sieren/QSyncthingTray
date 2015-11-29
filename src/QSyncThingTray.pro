HEADERS       = window.h \
                syncconnector.h \
                systemUtils.hpp \
                posixUtils.hpp \
                winUtils.hpp
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
macx {
QMAKE_INFO_PLIST = Info.plist
}
#QMAKE_CXXFLAGS += /wd4996
ICON = Syncthing.icns
macx {
APP_BINARY_FILES.files = resources/mac/syncthing-inotify
APP_BINARY_FILES.path = Contents/Resources
QMAKE_BUNDLE_DATA += APP_BINARY_FILES
}
