cmake_minimum_required(VERSION 3.0)
project(QSyncthingTray)

set(qst_src_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/qst)
set(contrib_src_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/contrib)
set(platforms_src_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/platforms)

set(qst_SOURCES
  ${qst_src_ROOT}/main.cpp
  ${qst_src_ROOT}/processcontroller.cpp
  ${qst_src_ROOT}/processmonitor.cpp
  ${qst_src_ROOT}/startuptab.cpp
  ${qst_src_ROOT}/statswidget.cpp
  ${qst_src_ROOT}/syncconnector.cpp
  ${qst_src_ROOT}/updatenotifier.cpp
  ${qst_src_ROOT}/window.cpp
  ${contrib_src_ROOT}/qcustomplot.cpp
)

if(APPLE)
set(qst_PLATFORMS_SOURCES
  ${platforms_src_ROOT}/darwin/macUtils.mm
)
endif()

if (${QST_BUILD_WEBKIT})
  set(qst_SOURCES
    ${qst_SOURCES}
    ${qst_src_ROOT}/syncwebkitview.cpp
  )
else()
  set(qst_SOURCES
    ${qst_SOURCES}
    ${qst_src_ROOT}/syncwebpage.cpp
    ${qst_src_ROOT}/syncwebview.cpp
  )
endif()

set(qst_SOURCES
  ${qst_SOURCES}
  PARENT_SCOPE
  )
set(qst_PLATFORMS_SOURCES
  ${qst_PLATFORMS_SOURCES}
  PARENT_SCOPE
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
source_group("Headers" FILES ${qst_HEADERS})
source_group("Sources" FILES ${qst_SOURCES})
source_group("Platforms Sources" FILES ${qst_PLATFORMS_SOURCES})
