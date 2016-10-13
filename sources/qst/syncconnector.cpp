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

namespace qst
{
namespace connector
{


//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
SyncConnector::SyncConnector(QUrl url, ConnectionStateCallback textCallback) :
    mConnectionStateCallback(textCallback)
  , mCurrentUrl(url)
  , mSettings("QSyncthingTray", "qst")
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
     mAuthentication));
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
    shutdownINotifyProcess();
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
    checkAndSpawnINotifyProcess(false);
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

void SyncConnector::syncThingProcessSpawned(QProcess::ProcessState newState)
{
  switch (newState)
  {
    case QProcess::Running:
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::SPAWNED}}));
      break;
    case QProcess::NotRunning:
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::NOT_RUNNING}}));
      break;
    default:
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::NOT_RUNNING}}));
  }
}


//------------------------------------------------------------------------------------//

void SyncConnector::notifyProcessSpawned(QProcess::ProcessState newState)
{
  switch (newState)
  {
    case QProcess::Running:
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::SPAWNED}}));
      break;
    case QProcess::NotRunning:
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::NOT_RUNNING}}));
      break;
    default:
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::NOT_RUNNING}}));
  }
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
  traffic.first = std::floor(traffic.first * 100) / 100;
  traffic.second = std::floor(traffic.second * 100) / 100;

  result.emplace("outTraffic", trafficToString(traffic.second));
  result.emplace("inTraffic", trafficToString(traffic.first));
  result.emplace("globalTraffic", trafficToString(traffic.first + traffic.second));

  emit(onNetworkActivityChanged(traffic.first + traffic.second > kNetworkNoiseFloor));
  emit(onConnectionHealthChanged(result));

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
  mSyncthingPaused = paused;
  if (paused)
  {
    shutdownSyncthingProcess();
    killProcesses();
  }
  else
  {
    spawnSyncthingProcess(mSyncthingFilePath, true);
    checkAndSpawnINotifyProcess(false);
    setURL(mCurrentUrl, mCurrentUrl.userName(),
     mCurrentUrl.password());
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
    1000 * mSettings.value("pollingInterval").toDouble());
  mConnectionHealthTime = mConnectionHealthTime == 0 ? 1000 : mConnectionHealthTime;
  mAPIKey = mSettings.value("apiKey").toString();
  mINotifyFilePath = mSettings.value("inotifypath").toString();
  mShouldLaunchINotify = mSettings.value("launchINotifyAtStartup").toBool();
}


//------------------------------------------------------------------------------------//

void SyncConnector::shutdownProcessPosted(QNetworkReply *reply)
{
  emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::PAUSED}}));
  reply->deleteLater();
}


//------------------------------------------------------------------------------------//

void SyncConnector::spawnSyncthingProcess(
  std::string filePath, const bool shouldSpawn, const bool onSetPath)
{
  mSyncthingFilePath = filePath;
  if (shouldSpawn)
  {
    if (!checkIfFileExists(tr(filePath.c_str())) && onSetPath)
    {
      QMessageBox msgBox;
      msgBox.setText("Could not find Syncthing.");
      msgBox.setInformativeText("Are you sure the path is correct?");
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      msgBox.exec();
    }
    if (!systemUtil.isBinaryRunning(std::string("syncthing")))
    {
      mpSyncProcess = std::unique_ptr<QProcess>(new QProcess(this));
      connect(mpSyncProcess.get(), SIGNAL(stateChanged(QProcess::ProcessState)),
        this, SLOT(syncThingProcessSpawned(QProcess::ProcessState)));
      QString processPath = QDir::toNativeSeparators(filePath.c_str());
      QStringList launchArgs;
      launchArgs << "-no-browser";
      mpSyncProcess->start(processPath, launchArgs);
    }
    else
    {
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::ALREADY_RUNNING}}));
    }
  }
}


//------------------------------------------------------------------------------------//

void SyncConnector::checkAndSpawnINotifyProcess(bool isRequestedExternal)
{
  if (isRequestedExternal)
  {
    onSettingsChanged();
    if (mpSyncthingNotifierProcess)
    {
      mpSyncthingNotifierProcess->terminate();
    }
  }
  if (mShouldLaunchINotify)
  {
    if (!checkIfFileExists(mINotifyFilePath) && isRequestedExternal)
    {
      QMessageBox msgBox;
      msgBox.setText("Could not find iNotify.");
      msgBox.setInformativeText("Are you sure the path is correct?");
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      msgBox.exec();
    }
    if (!systemUtil.isBinaryRunning(std::string("syncthing-inotify")))
    {
      mpSyncthingNotifierProcess = std::unique_ptr<QProcess>(new QProcess(this));
      QString processPath = QDir::toNativeSeparators(mINotifyFilePath);
      connect(mpSyncthingNotifierProcess.get(), SIGNAL(stateChanged(QProcess::ProcessState)),
        this, SLOT(notifyProcessSpawned(QProcess::ProcessState)));
      mpSyncthingNotifierProcess->start(processPath, QStringList(), QIODevice::Unbuffered);
    }
    else
    {
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::ALREADY_RUNNING}}));
    }
  }
  else
  {
    shutdownINotifyProcess();
  }
}


//------------------------------------------------------------------------------------//

void SyncConnector::shutdownINotifyProcess()
{
  if (mpSyncthingNotifierProcess != nullptr &&
      mpSyncthingNotifierProcess->state() == QProcess::Running)
  {
    mpSyncthingNotifierProcess->kill();
  }
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

auto SyncConnector::checkIfFileExists(QString path) -> bool
{
  QFileInfo checkFile(path);
  // check if file exists and if yes: Is it really a file and not a directory?
  if (checkFile.exists() && checkFile.isFile())
  {
    return true;
  }
  else
  {
    return false;
  }
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

void SyncConnector::killProcesses()
{
  if (mpSyncProcess != nullptr
      && mSettings.value("ShutdownOnExit").toBool())
  {
    mpSyncProcess->waitForFinished();
  }
  if (mpSyncthingNotifierProcess != nullptr
      && mpSyncthingNotifierProcess->state() == QProcess::Running)
  {
    mpSyncthingNotifierProcess->terminate();
    mpSyncthingNotifierProcess->waitForFinished();
  }
}


//------------------------------------------------------------------------------------//

auto SyncConnector::getWebView() -> webview::WebView *
{
  return mpSyncWebView.get();
}


//------------------------------------------------------------------------------------//

SyncConnector::~SyncConnector()
{
  if (mSettings.value("ShutdownOnExit").toBool())
  {
    shutdownSyncthingProcess();
  }
  mpConnectionHealthTimer->stop();
  killProcesses();
}

//------------------------------------------------------------------------------------//

template <typename T>
QString SyncConnector::trafficToString(T traffic)
{
  using namespace utilities;
  std::string strTraffic = traffic > kBytesToKilobytes ?
    to_string_with_precision(traffic/kBytesToKilobytes, 2) + " MB/s" :
    to_string_with_precision(traffic, 2) + " KB/s";
  return QString(strTraffic.c_str());
}


} // connector
} //qst
