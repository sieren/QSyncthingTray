cmake_minimum_required(VERSION 3.0)
project(QSyncthingTray)

#   ____
#  / ___|___  _ __ ___
# | |   / _ \| '__/ _ \
# | |__| (_) | | |  __/
#  \____\___/|_|  \___|
#

set(qst_thirdparty_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(qst_date_ROOT ${qst_thirdparty_ROOT}/date)
include_directories(date)
set(qst_date_FILES
  ${qst_date_ROOT}/date.h
)

set(qst_thirdparty_FILES
  ${qst_date_FILES}
  PARENT_SCOPE)

