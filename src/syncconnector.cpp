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

#include "syncconnector.h"
#include <QtGui>
#include <QObject>
#include <iostream>

namespace mfk
{
namespace connector
{

SyncConnector::SyncConnector(QUrl url)
{
  mCurrentUrl = url;
  connect(
          &mWebUrl, SIGNAL (finished(QNetworkReply*)),
          this, SLOT (urlTested(QNetworkReply*))
          );
  connect(
          &mWebUrl, SIGNAL (sslErrors(QNetworkReply *, QList<QSslError>)),
          this, SLOT (onSslError(QNetworkReply*))
          );
  
  connect(
          &mHealthUrl, SIGNAL (finished(QNetworkReply*)),
          this, SLOT (connectionHealthReceived(QNetworkReply*))
          );
  connect(
          &mHealthUrl, SIGNAL (sslErrors(QNetworkReply *, QList<QSslError>)),
          this, SLOT (onSslError(QNetworkReply*))
          );
  }

void SyncConnector::setURL(QUrl url, std::string username, std::string password, ConnectionStateCallback setText)
{

  mAuthentication = std::make_pair(username, password);
  url.setUserName(mAuthentication.first.c_str());
  url.setPassword(mAuthentication.second.c_str());
  mCurrentUrl = url;
  url.setPath(tr("/rest/system/version"));
  mConnectionStateCallback = setText;
  QNetworkRequest request(url);
  mWebUrl.clearAccessCache();
  mWebUrl.get(request);
}


void SyncConnector::showWebView()
{
  if (mpWebView != nullptr)
  {
    mpWebView->close();
  }
  std::unique_ptr<QWebView> pWeb(new QWebView());
  mpWebView = std::move(pWeb);
  mpWebView->show();
  mpWebView->page()->setNetworkAccessManager(&mWebUrl);
  mpWebView->load(mCurrentUrl);
  mpWebView->raise();
}

  
void SyncConnector::urlTested(QNetworkReply* reply)
{
  ignoreSslErrors(reply);
  std::string result;
  bool success = false;

  if (reply->error() != QNetworkReply::NoError)
  {
    result = reply->errorString().toStdString();
  }
  else
  {
    QString m_DownloadedData = static_cast<QString>(reply->readAll());
    QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
    QJsonObject replyData = replyDoc.object();
    result = replyData.value(tr("version")).toString().toStdString();
    success = true;
  }
  if (mConnectionStateCallback != nullptr)
  {
    mConnectionStateCallback(result, success);
  }
}


void SyncConnector::checkConnectionHealth()
{
  QUrl requestUrl = mCurrentUrl;
  requestUrl.setPath(tr("/rest/system/connections"));
  QNetworkRequest request(requestUrl);

  mHealthUrl.get(request);
}


void SyncConnector::setConnectionHealthCallback(ConnectionHealthCallback cb)
{
  mConnectionHealthCallback = cb;
  if (connectionHealthTimer)
  {
    connectionHealthTimer->stop();
  }
  connectionHealthTimer = std::shared_ptr<QTimer>(new QTimer(this));
  connect(connectionHealthTimer.get(), SIGNAL(timeout()), this, SLOT(checkConnectionHealth()));
  connectionHealthTimer->start(3000);
}


void SyncConnector::syncThingProcessSpawned(QProcess::ProcessState newState)
{
  if (mProcessSpawnedCallback)
  {
    switch (newState)
    {
      case QProcess::Running:
        mProcessSpawnedCallback(kSyncthingProcessState::SPAWNED);
        break;
      case QProcess::NotRunning:
         mProcessSpawnedCallback(kSyncthingProcessState::NOT_RUNNING);
        break;
      default:
        mProcessSpawnedCallback(kSyncthingProcessState::NOT_RUNNING);
    }
  }
}

  
void SyncConnector::setProcessSpawnedCallback(ProcessSpawnedCallback cb)
{
  mProcessSpawnedCallback = cb;
}


void SyncConnector::connectionHealthReceived(QNetworkReply* reply)
{
  ignoreSslErrors(reply);
  std::map<std::string, std::string> result;
  result.emplace("state", "0");
  if (reply->error() != QNetworkReply::NoError)
  {
  //  std::cout << "Failed: " << reply->error();
  //  result = reply->errorString().toStdString();
  }
  else
  {
    if (reply->bytesAvailable() > 0)
    {
      result.clear();
      result.emplace("state", "1");
      QString m_DownloadedData = static_cast<QString>(reply->readAll());
      QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
      QJsonObject replyData = replyDoc.object();
      QJsonObject connectionArray = replyData["connections"].toObject();
      result.emplace("connections", std::to_string(connectionArray.size()));
    }
  }
  if (mConnectionHealthCallback != nullptr)
  {
    mConnectionHealthCallback(result);
  }
}


void SyncConnector::spawnSyncthingProcess(std::string filePath)
{
  if (!systemUtil.isSyncthingRunning())
  {
    mpSyncProcess = new QProcess(this);
    connect(mpSyncProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(syncThingProcessSpawned(QProcess::ProcessState)));
    QString processPath = filePath.c_str();
    QStringList launchArgs;
    launchArgs << "-no-restart";
    launchArgs.append("-no-browser");
    mpSyncProcess->start(processPath, launchArgs);
  }
  else
  {
    if (mProcessSpawnedCallback != nullptr)
    {
      mProcessSpawnedCallback(kSyncthingProcessState::ALREADY_RUNNING);
    }
  }
}


void SyncConnector::ignoreSslErrors(QNetworkReply *reply)
{
  QList<QSslError> errorsThatCanBeIgnored;
  
  errorsThatCanBeIgnored<<QSslError(QSslError::HostNameMismatch);
  errorsThatCanBeIgnored<<QSslError(QSslError::SelfSignedCertificate);
  reply->ignoreSslErrors(errorsThatCanBeIgnored);
}

  
void SyncConnector::onSslError(QNetworkReply* reply)
{
  reply->ignoreSslErrors();
}

  
SyncConnector::~SyncConnector()
{
  mpSyncProcess->kill();
}
  
} // connector
} //mfk