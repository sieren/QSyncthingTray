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

#include <iostream>
#include <QStyleFactory>
#include <qst/platforms.hpp>
#include <qst/syncwebkitview.h>

//------------------------------------------------------------------------------------//
#define UNUSED(x) (void)(x)
//------------------------------------------------------------------------------------//

namespace qst
{
namespace webview
{

//------------------------------------------------------------------------------------//

SyncWebKitView::SyncWebKitView(const QUrl& url, const Authentication &authInfo)
  : mSyncThingUrl(url)
  , mAuthInfo(authInfo)
{
  
}


//------------------------------------------------------------------------------------//

void SyncWebKitView::show()
{
  if (mpWebView != nullptr)
  {
    mpWebView->close();
  }
  std::unique_ptr<QWebViewClose> pWeb(new QWebViewClose());
  mpWebView = std::move(pWeb);
  mpWebView->show();
  connect(mpWebView->page()->networkAccessManager(),
          SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )),
          this,
          SLOT(onSslError(QNetworkReply*)));
  mpWebView->load(mSyncThingUrl);
  mpWebView->setStyle(QStyleFactory::create("Fusion"));
  qst::sysutils::SystemUtility().showDockIcon(true);
  mpWebView->raise();
}


//------------------------------------------------------------------------------------//

void SyncWebKitView::onSslError(QNetworkReply* reply)
{
  reply->ignoreSslErrors();
}

//------------------------------------------------------------------------------------//

void SyncWebKitView::updateConnection(const QUrl &url, const Authentication &authInfo)
{
  mSyncThingUrl = url;
  mAuthInfo = authInfo;
}

//------------------------------------------------------------------------------------//

void QWebViewClose::closeEvent(QCloseEvent *event)
{
  UNUSED(event);
  qst::sysutils::SystemUtility().showDockIcon(false);
  close();
}


} // webview namespace
} // qst namespace