cmake_minimum_required(VERSION 3.2)

project (QSyncthingTray)

if ("$ENV{QTDIR}" STREQUAL "")
  MESSAGE(WARNING "QTDIR not specified. Please export QTDIR
  e.g. export QTDIR = ~/Qt/5.5/gcc_64/
  Will try to find system installation..")
endif ("$ENV{QTDIR}" STREQUAL "")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (${CMAKE_CL_64} MATCHES 0)
    message (STATUS "MSVC 64 Bit: ${CMAKE_CL_64}")
    set(CMAKE_PREFIX_PATH "$ENV{QTDIR}")
  else(${CMAKE_CL_64} MATCHES 0)
    set(CMAKE_PREFIX_PATH "$ENV{QTDIR}")
  endif(${CMAKE_CL_64} MATCHES 0)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")



# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network)

add_definitions(-DGLEW_STATIC)
# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

#  ____  _       _    __
# |  _ \| | __ _| |_ / _| ___  _ __ _ __ ___
# | |_) | |/ _` | __| |_ / _ \| '__| '_ ` _ \
# |  __/| | (_| | |_|  _| (_) | |  | | | | | |
# |_|   |_|\__,_|\__|_|  \___/|_|  |_| |_| |_|
#

set(qst_platform_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/includes/platforms)
set(qst_platform_SOURCES)
if(APPLE)
  set(qst_platform_SOURCES
    ${qst_platform_SOURCES}
    ${qst_platform_ROOT}/darwin/macUtils.hpp
)
elseif(WIN32)
  set(qst_platform_SOURCES
    ${qst_platform_SOURCES}
    ${qst_platform_ROOT}/windows/winUtils.hpp
)
elseif(UNIX)
  set(qst_platform_SOURCES
    ${qst_platform_SOURCES}
    ${qst_platform_ROOT}/linux/posixUtils.hpp
)
endif()
set(qst_platform_SOURCES
  ${qst_platform_SOURCES}
)

source_group("Platforms" ${qst_platform_SOURCES})

#  ____       _   _
# |  _ \ __ _| |_| |__  ___
# | |_) / _` | __| '_ \/ __|
# |  __/ (_| | |_| | | \__ \
# |_|   \__,_|\__|_| |_|___/
#

# qt5_wrap_ui(UI_HEADERS mainwindow.ui)
qt5_add_resources(UI_RESOURCES resources/qsyncthing.qrc)

include_directories(includes)
add_subdirectory(includes)
add_subdirectory(sources)

set(MACOSX_BUNDLE_ICON_FILE resources/Syncthing.icns)
SET_SOURCE_FILES_PROPERTIES(
  Syncthing.icns
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
  )

  if(APPLE)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    add_executable(QSyncthingTray MACOSX_BUNDLE resources/Syncthing.icns ${UI_HEADERS} ${UI_RESOURCES} 
    ${qst_platform_SOURCES} ${qst_HEADERS} ${qst_SOURCES})
  elseif(WIN32)
    add_executable(QSyncthingTray WIN32 ${UI_HEADERS} ${UI_RESOURCES} ${qst_platform_SOURCES} ${qst_HEADERS} ${qst_SOURCES})
  elseif(UNIX)
    add_executable(QSyncthingTray ${UI_HEADERS} ${UI_RESOURCES} ${qst_platform_SOURCES} ${qst_HEADERS} ${qst_SOURCES})
  endif()

if (APPLE)
  target_link_libraries(
      QSyncthingTray
      ${Qt5Core_QTMAIN_LIBRARIES}
      ${COCOA_LIBRARY})
else()
  target_link_libraries(
      QSyncthingTray
      ${Qt5Core_QTMAIN_LIBRARIES})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set_target_properties(QSyncthingTray PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(QSyncthingTray PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    target_link_libraries(QSyncthingTray ${Qt5Widgets_LIBRARIES} Qt5::Widgets) 
    get_target_property(QtCore_location Qt5::Core LOCATION)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (${QST_BUILD_WEBKIT})
  qt5_use_modules(QSyncthingTray Widgets Network WebKitWidgets)
  target_compile_definitions(QSyncthingTray PRIVATE BUILD_WEBKIT=1)
else()
  qt5_use_modules(QSyncthingTray Widgets Network WebEngineWidgets)
endif()

