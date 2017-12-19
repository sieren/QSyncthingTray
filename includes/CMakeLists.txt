
#   ____
#  / ___|___  _ __ ___
# | |   / _ \| '__/ _ \
# | |__| (_) | | |  __/
#  \____\___/|_|  \___|
#

set(qst_include_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/qst)
set(contrib_include_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/contrib)

set(qst_HEADERS
  ${qst_include_ROOT}/apihandler.hpp
  ${qst_include_ROOT}/appsettings.hpp
  ${qst_include_ROOT}/identifiers.hpp
  ${qst_include_ROOT}/platforms.hpp
  ${qst_include_ROOT}/processcontroller.h
  ${qst_include_ROOT}/processmonitor.hpp
  ${qst_include_ROOT}/settingsmigrator.hpp
  ${qst_include_ROOT}/startuptab.hpp
  ${qst_include_ROOT}/statswidget.h
  ${qst_include_ROOT}/syncconnector.h
  ${qst_include_ROOT}/updatenotifier.h
  ${qst_include_ROOT}/utilities.hpp
  ${qst_include_ROOT}/webview.h
  ${qst_include_ROOT}/window.h
  ${contrib_include_ROOT}/qcustomplot.h
)

if (${QST_BUILD_WEBKIT})
  set(qst_HEADERS
    ${qst_HEADERS}
    ${qst_include_ROOT}/syncwebkitview.h
  )
elseif (${QST_BUILD_NATIVEBROWSER})
  set(qst_HEADERS
    ${qst_HEADERS}
    ${qst_include_ROOT}/syncnativebrowser.h
  )
else()
  set(qst_HEADERS
    ${qst_HEADERS}
    ${qst_include_ROOT}/syncwebpage.h
    ${qst_include_ROOT}/syncwebview.h
  )
endif()

set(qst_HEADERS
  ${qst_HEADERS}
  PARENT_SCOPE)
