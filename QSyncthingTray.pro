HEADERS       = includes/qst/window.h \
                includes/qst/syncconnector.h \
                includes/platforms/darwin/macUtils.hpp \
                includes/platforms/windows/winUtils.hpp \
                includes/platforms/linux/posixUtils.hpp \
                includes/qst/processmonitor.hpp \
                includes/qst/platforms.hpp \
                includes/qst/apihandler.hpp \
                includes/qst/startuptab.hpp \
                includes/qst/utilities.hpp \
                includes/qst/syncwebview.h \
                includes/qst/syncwebpage.h
SOURCES       = sources/qst/main.cpp \
                sources/qst/window.cpp \
                sources/qst/syncconnector.cpp \
                sources/qst/processmonitor.cpp \
                sources/qst/startuptab.cpp \
                sources/qst/syncwebview.cpp \
                sources/qst/syncwebpage.cpp
RESOURCES     = \
                resources/qsyncthing.qrc

QT += widgets
QT += network
QT += webenginewidgets
INCLUDEPATH += $$PWD/includes/
# install
target.path = binary/
INSTALLS += target
CONFIG += c++11
macx {
QMAKE_INFO_PLIST = Info.plist
LIBS += -framework ApplicationServices
}
#QMAKE_CXXFLAGS += /wd4996
ICON = resources/Syncthing.icns
macx {
APP_BINARY_FILES.path = Contents/Resources
QMAKE_BUNDLE_DATA += APP_BINARY_FILES
}
