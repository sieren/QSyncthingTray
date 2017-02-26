cmake_minimum_required(VERSION 3.2)

project (QSyncthingTray LANGUAGES CXX)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Qt5 5.6 COMPONENTS Widgets Network PrintSupport)
if (NOT Qt5_FOUND)
  message(FATAL_ERROR
    "Some components of Qt5 not found (see above messages for details. "
    "Did you set CMAKE_PREFIX_PATH ?\n"
    "Example: cmake -D CMAKE_PREFIX_PATH=/my/path/to/Qt5/5.7/clang_64 .."
  )
endif ()

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
set(qst_platform_HEADERS)
if(APPLE)
  set(qst_platform_HEADERS
    ${qst_platform_HEADERS}
    ${qst_platform_ROOT}/darwin/macUtils.hpp
)
elseif(WIN32)
  set(qst_platform_HEADERS
    ${qst_platform_HEADERS}
    ${qst_platform_ROOT}/windows/winUtils.hpp
)
elseif(UNIX)
  set(qst_platform_HEADERS
    ${qst_platform_HEADERS}
    ${qst_platform_ROOT}/linux/posixUtils.hpp
)
endif()
set(qst_platform_HEADERS
  ${qst_platform_HEADERS}
)

source_group("Platforms" ${qst_platform_HEADERS} ${qst_PLATFORMS_SOURCES})

#  ____       _   _
# |  _ \ __ _| |_| |__  ___
# | |_) / _` | __| '_ \/ __|
# |  __/ (_| | |_| | | \__ \
# |_|   \__,_|\__|_| |_|___/
#

# qt5_wrap_ui(UI_HEADERS mainwindow.ui)
qt5_add_resources(UI_RESOURCES
  resources/qsyncthing.qrc
  resources/qsyncthingblueanim.qrc
  resources/qsyncthingdarkanim.qrc)

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
      ${qst_platform_HEADERS} ${qst_PLATFORMS_SOURCES} ${qst_HEADERS} ${qst_SOURCES})
  elseif(WIN32)
    add_executable(QSyncthingTray WIN32 ${UI_HEADERS} ${UI_RESOURCES} ${qst_platform_HEADERS} ${qst_HEADERS} ${qst_SOURCES})
  elseif(UNIX)
    add_executable(QSyncthingTray ${UI_HEADERS} ${UI_RESOURCES} ${qst_platform_HEADERS} ${qst_HEADERS} ${qst_SOURCES})
  endif()

if (APPLE)
  target_link_libraries(
      QSyncthingTray
      ${Qt5Core_QTMAIN_LIBRARIES}
      ${COCOA_LIBRARY}
      Qt5::PrintSupport)
else()
  target_link_libraries(
      QSyncthingTray
      ${Qt5Core_QTMAIN_LIBRARIES}
      Qt5::PrintSupport)
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


# Temporary solution/hack to generate package.
# Proper way will come after cmake cleanup.

get_target_property(qt5_qmake Qt5::qmake LOCATION)
get_filename_component(qt5_bin_dir ${qt5_qmake} DIRECTORY)

if(APPLE)
  find_program(qt5_qtdeploy
      NAMES macdeployqt
      PATHS ${qt5_bin_dir})
  if(NOT qt5_qtdeploy)
    message(FATAL_ERROR "macdeployqt not found")
  endif()

  add_custom_target(create_package
      COMMAND ${CMAKE_COMMAND} -E remove_directory QSyncthingTray.app
      COMMAND ${CMAKE_COMMAND} -E remove QSyncthingTray.dmg
      COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG>
      COMMAND ${qt5_qtdeploy} QSyncthingTray.app -verbose=1 -dmg
      COMMENT "Creating package")
elseif(WIN32)
  find_program(qt5_qtdeploy
      NAMES windeployqt
      PATHS ${qt5_bin_dir})
  if(NOT qt5_qtdeploy)
    message(FATAL_ERROR "windeployqt not found")
  endif()

  add_custom_target(create_package
      COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG>
      COMMAND ${qt5_qtdeploy} --compiler-runtime $<CONFIG>/QSyncthingTray.exe
      COMMENT "Creating package")
endif()
