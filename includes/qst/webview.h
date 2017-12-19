/******************************************************************************
 // QSyncthingTray
 // Copyright (c) Matthias Frick, All rights reserved.
 //
 // This library is free software; you can redistribute it and/or
 // modify it under the terms of the GNU Lesser General Public
 // License as published by the Free Software Foundation; either
 // version 3.0 of the License, or (at your option) any later version.
 //
 // This library is distributed in the hope that it will be useful,
 // but WITHOUT ANY WARRANTY; without even the implied warranty of
 // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 // Lesser General Public License for more details.
 //
 // You should have received a copy of the GNU Lesser General Public
 // License along with this library.
 ******************************************************************************/

#pragma once

#ifdef BUILD_WEBKIT
#include <qst/syncwebkitview.h>
#elif defined(BUILD_NATIVEBROWSER)
#include <qst/syncnativebrowser.h>
#else
#include <qst/syncwebview.h>
#include <qst/syncwebpage.h>
#endif

namespace qst
{
namespace webview
{

#ifdef BUILD_WEBKIT
using WebView = qst::webview::SyncWebKitView;
#elif defined(BUILD_NATIVEBROWSER)
using WebView = qst::webview::SyncNativeBrowser;
#else
using WebView = qst::webview::SyncWebView;
#endif

} // webview
} // qst
