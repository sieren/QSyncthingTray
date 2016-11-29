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


#ifndef SYNCWEBKITVIEW_H
#define SYNCWEBKITVIEW_H

//------------------------------------------------------------------------------------//

#include <QNetworkReply>
#include <QWebView>
#include <qst/appsettings.hpp>
#include <memory>

//------------------------------------------------------------------------------------//

using Authentication = std::pair<QString, QString>;

//------------------------------------------------------------------------------------//

namespace qst
{
namespace webview
{

//------------------------------------------------------------------------------------//

class QWebViewClose;

//------------------------------------------------------------------------------------//

static const bool kWebViewSupportsZoom = false;

//------------------------------------------------------------------------------------//

class SyncWebKitView : public QWidget
{
  Q_OBJECT

public:
  SyncWebKitView() = default;
  SyncWebKitView(const QUrl &url, const Authentication &authInfo,
                 std::shared_ptr<settings::AppSettings> /*appSetting*/);
  ~SyncWebKitView() = default;

  void show();
  void updateConnection(const QUrl& url, const Authentication& authInfo);
  void setZoomFactor(const qreal factor) { }
  
signals:
  void close();

private slots:
  void onSslError(QNetworkReply* reply);

private:
  std::unique_ptr<QWebViewClose> mpWebView;
  QUrl mSyncThingUrl;
  Authentication mAuthInfo;
};

class QWebViewClose : public QWebView
{
  Q_OBJECT;
  public slots:
  void closeEvent(QCloseEvent *event) override;
};

} // webview namespace
} // qst namespace

#endif

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
// EOF
//------------------------------------------------------------------------------------//
