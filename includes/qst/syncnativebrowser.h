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

#ifndef SYNCNATIVEBROWSER_H
#define SYNCNATIVEBROWSER_H

#include <QObject>
#include <QUrl>
#include <memory>
#include "utilities.hpp"
#include <qst/appsettings.hpp>
#include <qst/syncwebbase.h>

//------------------------------------------------------------------------------------//

namespace qst
{
namespace webview
{

//------------------------------------------------------------------------------------//

static const bool kWebViewSupportsZoom = false;

//------------------------------------------------------------------------------------//

class SyncNativeBrowser : public QObject
{
  Q_OBJECT;
public:
  SyncNativeBrowser() = default;
  SyncNativeBrowser(QUrl url,
              Authentication authInfo,
              std::shared_ptr<settings::AppSettings> appSettings);
  ~SyncNativeBrowser() = default;
  
  void show();
  bool isVisible() const; // NOOP
  void raise(); // NOOP
  void setZoomFactor(double value); // NOOP
  void updateConnection(const QUrl &url, const Authentication &authInfo);

signals:
  void close();

private:
  QUrl mSyncThingUrl;
  Authentication mAuthInfo;
  std::shared_ptr<settings::AppSettings> mpAppSettings;
};

  
} // qst namespace
} // webview namespace

#endif // SYNCNATIVEBROWSER_H

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
// EOF
//------------------------------------------------------------------------------------//
