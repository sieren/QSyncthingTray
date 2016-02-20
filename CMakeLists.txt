cmake_minimum_required(VERSION 3.2)

project (QSyncthingTray)

if ("$ENV{QTDIR}" STREQUAL "")
  MESSAGE(FATAL_ERROR "QTDIR not specified. Please export QTDIR 
  e.g. export QTDIR = ~/Qt/5.5/gcc_64/")
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
find_package(Qt5Declarative)

add_definitions(-DGLEW_STATIC)
# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

# qt5_wrap_ui(UI_HEADERS mainwindow.ui)
qt5_add_resources(UI_RESOURCES src/qsyncthing.qrc)

add_subdirectory(src src-binary)
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE FILES_WP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRC_ROOT}/*.cpp ${SRC_ROOT}/*.h ${SRC_ROOT}/*.hpp)



set(MACOSX_BUNDLE_ICON_FILE Syncthing.icns)
SET_SOURCE_FILES_PROPERTIES(
  Syncthing.icns
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
  )

  if(APPLE)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    add_executable(QSyncthingTray MACOSX_BUNDLE src/Syncthing.icns ${UI_HEADERS} ${UI_RESOURCES} ${FILES_WP})
  elseif(WIN32)
    add_executable(QSyncthingTray WIN32 ${UI_HEADERS} ${UI_RESOURCES} ${FILES_WP})
  elseif(UNIX)
    add_executable(QSyncthingTray ${UI_HEADERS} ${UI_RESOURCES} ${FILES_WP})
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

qt5_use_modules(QSyncthingTray Widgets Network WebEngineWidgets)

