cmake_minimum_required(VERSION 3.2)

project (QSyncthingTray)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.5\\msvc2013_64")
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_PREFIX_PATH "~/Qt/5.5/clang_64/")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5WebKitWidgets)
find_package(Qt5Declarative)

add_definitions(-DGLEW_STATIC)


# qt5_wrap_ui(UI_HEADERS mainwindow.ui)
qt5_add_resources(UI_RESOURCES src/qsyncthing.qrc)

add_subdirectory(src src-binary)
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB FILES_WP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRC_ROOT}/*.cpp ${SRC_ROOT}/*.h ${SRC_ROOT}/*.hpp)



set(MACOSX_BUNDLE_ICON_FILE Syncthing.icns)
SET_SOURCE_FILES_PROPERTIES(
  Syncthing.icns
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
  )


  if(APPLE)
    add_executable(QSyncthingTray MACOSX_BUNDLE src/Syncthing.icns ${UI_HEADERS} ${UI_RESOURCES} ${FILES_WP})
  elseif(WIN32)
    add_executable(QSyncthingTray WIN32 ${FILES_WP})
  elseif(UNIX)
    add_executable(QSyncthingTray ${FILES_WP})
  endif()
  
target_link_libraries(
    QSyncthingTray
    ${Qt5Core_QTMAIN_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set_target_properties(BSEd PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(BSEd PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

qt5_use_modules(QSyncthingTray Widgets Network WebKitWidgets)

