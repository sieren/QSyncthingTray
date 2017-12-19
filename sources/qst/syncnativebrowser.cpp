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


#include <qst/syncnativebrowser.h>
#include <QContextMenuEvent>
#include <functional>
#include <QDesktopServices>


//------------------------------------------------------------------------------------//
#define UNUSED(x) (void)(x)
//------------------------------------------------------------------------------------//

namespace qst
{
namespace webview
{

//------------------------------------------------------------------------------------//


SyncNativeBrowser::SyncNativeBrowser(QUrl url, Authentication authInfo,
                         std::shared_ptr<settings::AppSettings> appSettings) :
mSyncThingUrl(url)
,mAuthInfo(authInfo)
,mpAppSettings(appSettings)
{
}

//------------------------------------------------------------------------------------//

void SyncNativeBrowser::updateConnection(const QUrl &url, const Authentication &authInfo)
{
  mSyncThingUrl = url;
  mAuthInfo = authInfo;
}


//------------------------------------------------------------------------------------//

bool SyncNativeBrowser::isVisible() const
{
  return false;
}


//------------------------------------------------------------------------------------//

void SyncNativeBrowser::setZoomFactor(double value)
{
  UNUSED(value);
// NOOP
}


//------------------------------------------------------------------------------------//
  
void SyncNativeBrowser::raise()
{
// NOOP
}
 

//------------------------------------------------------------------------------------//

void SyncNativeBrowser::show()
{
  QDesktopServices::openUrl(QUrl(mSyncThingUrl));
}

} // qst namespace
} // webview namespace

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
// EOF
//------------------------------------------------------------------------------------//
