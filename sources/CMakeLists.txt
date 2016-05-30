cmake_minimum_required(VERSION 3.0)
project(QSyncthingTray)

set(qst_src_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/qst)

set(qst_SOURCES
  ${qst_src_ROOT}/main.cpp
  ${qst_src_ROOT}/processmonitor.cpp
  ${qst_src_ROOT}/startuptab.cpp
  ${qst_src_ROOT}/syncconnector.cpp
  ${qst_src_ROOT}/syncwebpage.cpp
  ${qst_src_ROOT}/syncwebview.cpp
  ${qst_src_ROOT}/window.cpp
  PARENT_SCOPE
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
source_group("Headers" FILES ${qst_HEADERS})
source_group("Sources" FILES ${qst_SOURCES})