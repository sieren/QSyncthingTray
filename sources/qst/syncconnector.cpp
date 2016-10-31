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

#include <qst/syncconnector.h>
#include <QtGui>
#include <QObject>
#include <QMessageBox>
#include <QStyleFactory>
#include <cmath>
#include <iostream>
#include <qst/platforms.hpp>
#include <qst/utilities.hpp>
#include <qst/processcontroller.h>

namespace qst
{
namespace connector
{


//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
SyncConnector::SyncConnector(QUrl url, ConnectionStateCallback textCallback,
  std::shared_ptr<settings::AppSettings> appSettings) :
    mConnectionStateCallback(textCallback)
  , mCurrentUrl(url)
  , mpAppSettings(appSettings)
{
  onSettingsChanged();
  connect(
          &network, SIGNAL (finished(QNetworkReply*)),
          this, SLOT (netRequestfinished(QNetworkReply*))
          );
  connect(
          &network, SIGNAL (sslErrors(QNetworkReply *, QList<QSslError>)),
          this, SLOT (onSslError(QNetworkReply*))
          );
  connect(mpAppSettings.get(), &settings::AppSettings::settingsUpdated,
          this, &SyncConnector::onSettingsChanged);
  mpConnectionHealthTimer = std::unique_ptr<QTimer>(new QTimer(this));
  mpConnectionAvailabilityTimer = std::unique_ptr<QTimer>(new QTimer(this));
  connect(mpConnectionHealthTimer.get(), SIGNAL(timeout()), this,
          SLOT(checkConnectionHealth()));
  connect(mpConnectionAvailabilityTimer.get(), SIGNAL(timeout()), this,
          SLOT(testUrlAvailability()));
}


//------------------------------------------------------------------------------------//

void SyncConnector::setURL(QUrl url, const QString& username, const
  QString& password)
{
  if (username.isEmpty() == false && password.isEmpty() == false)
  {
    mAuthentication = std::make_pair(username, password);
    url.setUserName(mAuthentication.first);
    url.setPassword(mAuthentication.second);
  }
  mCurrentUrl = url;
  testUrlAvailability();
}


//------------------------------------------------------------------------------------//

void SyncConnector::testUrlAvailability()
{
  QUrl url = mCurrentUrl;
  url.setPath(tr("/rest/system/version"));
  QNetworkRequest request(url);
  QByteArray headerByte(mAPIKey.toStdString().c_str(), mAPIKey.size());
  request.setRawHeader(QByteArray("X-API-Key"), headerByte);
  network.clearAccessCache();
  QNetworkReply *reply = network.get(request);
  requestMap[reply] = kRequestMethod::urlTested;
  if (mpSyncWebView != nullptr)
  {
    mpSyncWebView->updateConnection(mCurrentUrl, mAuthentication);
  }
  didShowSSLWarning = false;
}


//------------------------------------------------------------------------------------//

void SyncConnector::showWebView()
{
  if (mpSyncWebView != nullptr && mpSyncWebView->isVisible())
  {
    mpSyncWebView->raise();
    return;
  }

  mpSyncWebView = std::unique_ptr<webview::WebView>(new webview::WebView(mCurrentUrl,
     mAuthentication, mpAppSettings));
  connect(mpSyncWebView.get(), &webview::WebView::close, this, &SyncConnector::webViewClosed);
  mpSyncWebView->show();
}


//------------------------------------------------------------------------------------//

void SyncConnector::webViewClosed()
{
  disconnect(mpSyncWebView.get(), &webview::WebView::close,
    this, &SyncConnector::webViewClosed);
  mpSyncWebView->deleteLater();
  mpSyncWebView.release();
}


//------------------------------------------------------------------------------------//

void SyncConnector::urlTested(QNetworkReply* reply)
{
  ignoreSslErrors(reply);
  if (reply->error() == QNetworkReply::TimeoutError ||
      reply->error() == QNetworkReply::ConnectionRefusedError)
  {
    mpConnectionAvailabilityTimer->start(1000);
  }
  else
  {
    ConnectionState connectionInfo =
      api::APIHandlerFactory<QNetworkReply>().getConnectionVersionInfo(reply);

    int versionNumber = getCurrentVersion(connectionInfo.first);
    if (mAPIHandler == nullptr || mAPIHandler->version != versionNumber)
    {
      mAPIHandler =
        std::unique_ptr<api::APIHandlerBase>(
          api::APIHandlerFactory<QNetworkReply>().getAPIForVersion(versionNumber));
    }

    mConnectionStateCallback(connectionInfo);
    mpConnectionAvailabilityTimer->stop();
    mpConnectionHealthTimer->start(mConnectionHealthTime);
  }
  reply->deleteLater();
}


//------------------------------------------------------------------------------------//

void SyncConnector::checkConnectionHealth()
{
  QUrl requestUrl = mCurrentUrl;
  requestUrl.setPath(tr("/rest/system/connections"));
  QNetworkRequest healthRequest(requestUrl);
  QByteArray headerByte(mAPIKey.toStdString().c_str(), mAPIKey.size());
  healthRequest.setRawHeader(QByteArray("X-API-Key"), headerByte);
  QNetworkReply *reply = network.get(healthRequest);
  requestMap[reply] = kRequestMethod::connectionHealth;

  QUrl lastSyncedListURL = mCurrentUrl;
  lastSyncedListURL.setPath(tr("/rest/stats/folder"));
  QNetworkRequest lastSyncedRequest(lastSyncedListURL);
  lastSyncedRequest.setRawHeader(QByteArray("X-API-Key"), headerByte);
  QNetworkReply *lastSyncreply = network.get(lastSyncedRequest);
  requestMap[lastSyncreply] = kRequestMethod::getLastSyncedFiles;

  getCurrentConfig();
}


//------------------------------------------------------------------------------------//

void SyncConnector::getCurrentConfig()
{
  QUrl requestUrl = mCurrentUrl;
  requestUrl.setPath(tr("/rest/system/config"));
  QNetworkRequest request(requestUrl);
  QByteArray headerByte(mAPIKey.toStdString().c_str(), mAPIKey.size());
  request.setRawHeader(QByteArray("X-API-Key"), headerByte);
  QNetworkReply *reply = network.get(request);
  requestMap[reply] = kRequestMethod::getCurrentConfig;
}




//------------------------------------------------------------------------------------//

void SyncConnector::netRequestfinished(QNetworkReply* reply)
{
  switch (requestMap[reply])
  {
    case kRequestMethod::getCurrentConfig:
      currentConfigReceived(reply);
      break;
    case kRequestMethod::connectionHealth:
      connectionHealthReceived(reply);
      break;
    case kRequestMethod::urlTested:
      urlTested(reply);
      break;
    case kRequestMethod::getLastSyncedFiles:
      lastSyncedFilesReceived(reply);
      break;
    case kRequestMethod::shutdownRequested:
      shutdownProcessPosted(reply);
      break;
  }
  requestMap.remove(reply);
}


//------------------------------------------------------------------------------------//

void SyncConnector::connectionHealthReceived(QNetworkReply* reply)
{
  ignoreSslErrors(reply);
  QByteArray replyData;
  if (reply->error() == QNetworkReply::NoError)
  {
    replyData = reply->readAll();
  }
  auto result = mAPIHandler->getConnections(replyData);
  auto traffic = mAPIHandler->getCurrentTraffic(replyData);

  emit(onNetworkActivityChanged(std::get<0>(traffic) + std::get<1>(traffic) > kNetworkNoiseFloor));
  emit(onConnectionHealthChanged({result, traffic}));

  reply->deleteLater();
}


//------------------------------------------------------------------------------------//

void SyncConnector::currentConfigReceived(QNetworkReply *reply)
{
  ignoreSslErrors(reply);
  QByteArray replyData;
  if (reply->error() == QNetworkReply::NoError)
  {
    replyData = reply->readAll();
  }
  mFolders = mAPIHandler->getCurrentFolderList(replyData);
  reply->deleteLater();
}


//------------------------------------------------------------------------------------//

void SyncConnector::lastSyncedFilesReceived(QNetworkReply *reply)
{
  QByteArray replyData;
  if (reply->error() == QNetworkReply::NoError)
  {
    replyData = reply->readAll();
  }
  mLastSyncedFiles = mAPIHandler->getLastSyncedFiles(replyData);
  reply->deleteLater();
}


//------------------------------------------------------------------------------------//

LastSyncedFileList SyncConnector::getLastSyncedFiles()
{
  return mLastSyncedFiles;
}


//------------------------------------------------------------------------------------//

void SyncConnector::pauseSyncthing(bool paused)
{
  if (paused)
  {
    shutdownSyncthingProcess();
  }
}


//------------------------------------------------------------------------------------//

void SyncConnector::shutdownSyncthingProcess()
{
  QUrl requestUrl = mCurrentUrl;
  requestUrl.setPath(tr("/rest/system/shutdown"));
  QNetworkRequest request(requestUrl);
  QByteArray postData;
  // Call the webservice
  QNetworkRequest networkRequest(requestUrl);
  QByteArray headerByte(mAPIKey.toStdString().c_str(), mAPIKey.size());
  networkRequest.setRawHeader(QByteArray("X-API-Key"), headerByte);
  QNetworkReply *reply = network.post(networkRequest, postData);
  requestMap[reply] = kRequestMethod::shutdownRequested;
  if (!mSyncthingPaused)
  {
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
  }
}


//------------------------------------------------------------------------------------//

void SyncConnector::onSettingsChanged()
{
  mConnectionHealthTime = std::round(
    1000 * mpAppSettings->value(kPollingIntervalId).toDouble());
  mConnectionHealthTime = mConnectionHealthTime == 0 ? 1000 : mConnectionHealthTime;
  mAPIKey = mpAppSettings->value(kApiKeyId).toString();
  mINotifyFilePath = mpAppSettings->value(kInotifyPathId).toString();
  mShouldLaunchINotify = mpAppSettings->value(kLaunchInotifyStartupId).toBool();
}


//------------------------------------------------------------------------------------//

void SyncConnector::shutdownProcessPosted(QNetworkReply *reply)
{
  reply->deleteLater();
}


//------------------------------------------------------------------------------------//

auto SyncConnector::getFolders() -> std::list<FolderNameFullPath>
{
  return mFolders;
}


//------------------------------------------------------------------------------------//

void SyncConnector::ignoreSslErrors(QNetworkReply *reply)
{
  QList<QSslError> errorsThatCanBeIgnored;
  size_t foundHttp = mCurrentUrl.toString().toStdString().find("http:");
  QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

  // we're getting redirected, find out if to HTTPS
  if (statusCode.toInt() == 302 || statusCode.toInt() == 307)
  {
    QVariant url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    size_t found = url.toString().toStdString().find("https:");
    if (found != std::string::npos && foundHttp != std::string::npos
      && !didShowSSLWarning)
    {
      QMessageBox *msgBox = new QMessageBox;
      msgBox->setText("SSL Warning");
      msgBox->setInformativeText("The SyncThing Server seems to have HTTPS activated, "
        "however you are using HTTP. Please make sure to use a correct URL.");
      msgBox->setStandardButtons(QMessageBox::Ok);
      msgBox->setDefaultButton(QMessageBox::Ok);
      msgBox->setAttribute(Qt::WA_DeleteOnClose);
      msgBox->show();
      msgBox->setFocus();
      mCurrentUrl = QUrl(mCurrentUrl.toString().replace(tr("http"), tr("https")));
      didShowSSLWarning = true;
    }
  }

  errorsThatCanBeIgnored<<QSslError(QSslError::HostNameMismatch);
  errorsThatCanBeIgnored<<QSslError(QSslError::SelfSignedCertificate);
  reply->ignoreSslErrors();
}


//------------------------------------------------------------------------------------//

void SyncConnector::onSslError(QNetworkReply* reply)
{
  reply->ignoreSslErrors();
}


//------------------------------------------------------------------------------------//

auto SyncConnector::getCurrentVersion(QString reply) -> int
{
  std::string replyStd = reply.toStdString();
  std::string separator(".");
  std::size_t pos1 = replyStd.find(separator);
  std::size_t pos2 = replyStd.find(separator, pos1+1);
  std::string result = replyStd.substr (pos1+1, pos2-pos1-1);
  int version = 0;
  try
  {
    version = std::stoi(result);
  }
  catch (std::exception &e)
  {
    std::cerr << "Error getting current version: No or invalid connection."
      << std::endl;
  }
  return version;
}


//------------------------------------------------------------------------------------//

auto SyncConnector::getWebView() -> webview::WebView *
{
  return mpSyncWebView.get();
}


//------------------------------------------------------------------------------------//

SyncConnector::~SyncConnector()
{
  if (mpAppSettings->value("ShutdownOnExit").toBool())
  {
    shutdownSyncthingProcess();
  }
  mpConnectionHealthTimer->stop();
}


} // connector
} //qst
