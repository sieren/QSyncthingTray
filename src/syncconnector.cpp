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
#include <QMessageBox>
#include <iostream>

namespace mfk
{
namespace connector
{

  
//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
SyncConnector::SyncConnector(QUrl url)
{
  mCurrentUrl = url;
  connect(
          &network, SIGNAL (finished(QNetworkReply*)),
          this, SLOT (netRequestfinished(QNetworkReply*))
          );
  connect(
          &network, SIGNAL (sslErrors(QNetworkReply *, QList<QSslError>)),
          this, SLOT (onSslError(QNetworkReply*))
          );
}

  
//------------------------------------------------------------------------------------//

void SyncConnector::setURL(QUrl url, std::string username, std::string password,
  ConnectionStateCallback setText)
{
  mAuthentication = std::make_pair(username, password);
  url.setUserName(mAuthentication.first.c_str());
  url.setPassword(mAuthentication.second.c_str());
  mCurrentUrl = url;
  url.setPath(tr("/rest/system/version"));
  mConnectionStateCallback = setText;
  QNetworkRequest request(url);
  network.clearAccessCache();
  QNetworkReply *reply = network.get(request);
  requestMap[reply] = kRequestMethod::urlTested;
}


//------------------------------------------------------------------------------------//

void SyncConnector::showWebView()
{
  if (mpWebView != nullptr)
  {
    mpWebView->close();
  }
  std::unique_ptr<QWebView> pWeb(new QWebView());
  mpWebView = std::move(pWeb);
  mpWebView->show();
  mpWebView->page()->setNetworkAccessManager(&network);
  mpWebView->load(mCurrentUrl);
  mpWebView->raise();
}


//------------------------------------------------------------------------------------//

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


//------------------------------------------------------------------------------------//

void SyncConnector::checkConnectionHealth()
{
  QUrl requestUrl = mCurrentUrl;
  requestUrl.setPath(tr("/rest/system/connections"));
  QNetworkRequest request(requestUrl);

  QNetworkReply *reply = network.get(request);
  requestMap[reply] = kRequestMethod::connectionHealth;
  getCurrentConfig();
}


//------------------------------------------------------------------------------------//

void SyncConnector::getCurrentConfig()
{
  QUrl requestUrl = mCurrentUrl;
  requestUrl.setPath(tr("/rest/system/config"));
  QNetworkRequest request(requestUrl);
  
  QNetworkReply *reply = network.get(request);
  requestMap[reply] = kRequestMethod::getCurrentConfig;
}


//------------------------------------------------------------------------------------//

void SyncConnector::setConnectionHealthCallback(ConnectionHealthCallback cb)
{
  mConnectionHealthCallback = cb;
  if (mpConnectionHealthTimer)
  {
    mpConnectionHealthTimer->stop();
  }
  mpConnectionHealthTimer = std::unique_ptr<QTimer>(new QTimer(this));
  connect(mpConnectionHealthTimer.get(), SIGNAL(timeout()), this,
    SLOT(checkConnectionHealth()));
  mpConnectionHealthTimer->start(3000);
}


//------------------------------------------------------------------------------------//

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

 
//------------------------------------------------------------------------------------//

void SyncConnector::setProcessSpawnedCallback(ProcessSpawnedCallback cb)
{
  mProcessSpawnedCallback = cb;
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
  }
  requestMap.remove(reply);
}


//------------------------------------------------------------------------------------//

void SyncConnector::connectionHealthReceived(QNetworkReply* reply)
{
  ignoreSslErrors(reply);
  std::map<std::string, std::string> result;
  result.emplace("state", "0");
  if (reply->error() != QNetworkReply::NoError)
  {
  //  TODO: Error Handling
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
      int active = 0;
      for (QJsonObject::Iterator it = connectionArray.begin();
           it != connectionArray.end(); it++)
      {
        QJsonObject jObj = it->toObject();
        active += jObj.find("connected").value().toBool() ? 1 : 0;
      }
      result.emplace("activeConnections", std::to_string(active));
      result.emplace("totalConnections", std::to_string(connectionArray.size()));
    }
  }
  if (mConnectionHealthCallback != nullptr)
  {
    mConnectionHealthCallback(result);
  }
}


//------------------------------------------------------------------------------------//

void SyncConnector::currentConfigReceived(QNetworkReply *reply)
{
  ignoreSslErrors(reply);
  std::list<std::pair<std::string, std::string>> result;
  if (reply->error() != QNetworkReply::NoError)
  {
    // do nothing for now
  }
  else
  {
    if (reply->bytesAvailable() > 0)
    {
      QString m_DownloadedData = static_cast<QString>(reply->readAll());
      QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
      QJsonObject replyData = replyDoc.object();
      QJsonObject guiData = replyData["gui"].toObject();
      mAPIKey =  guiData["apiKey"].toString().toStdString();
      QJsonArray foldersArray = replyData["folders"].toArray();
      QJsonArray::iterator it;
      foreach (const QJsonValue & value, foldersArray)
      {
        std::pair<std::string, std::string> aResult;
        QJsonObject singleEntry = value.toObject();
        aResult.first = singleEntry.find("id").value().toString().toStdString();
        aResult.second = singleEntry.find("path").value().toString().toStdString();
        result.emplace_back(aResult);
      }
      mFolders = result;
    }
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
  QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
  connect(networkManager, SIGNAL(finished(QNetworkReply*)),
          SLOT(shutdownProcessPosted(QNetworkReply*)));
  connect(networkManager, SIGNAL (sslErrors(QNetworkReply *, QList<QSslError>)),
          this, SLOT (onSslError(QNetworkReply*))
          );
  QNetworkRequest networkRequest(requestUrl);
  std::string headerStr = "X-API-KEY: " + mAPIKey;
  std::cout << headerStr << std::endl;
  QByteArray headerByte(mAPIKey.c_str(), mAPIKey.length());
  networkRequest.setRawHeader("X-API-Key", headerByte);
  
  networkManager->post(networkRequest,postData);
}

//------------------------------------------------------------------------------------//

void SyncConnector::shutdownProcessPosted(QNetworkReply *reply)
{
#pragma unused(reply)
  if (mProcessSpawnedCallback)
  {
    mProcessSpawnedCallback(kSyncthingProcessState::PAUSED);
  }
}


//------------------------------------------------------------------------------------//

void SyncConnector::spawnSyncthingProcess(std::string filePath, const bool onSetPath)
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
    QString processPath = filePath.c_str();
    QStringList launchArgs;
    launchArgs << "-no-browser";
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


//------------------------------------------------------------------------------------//

std::list<std::pair<std::string, std::string>> SyncConnector::getFolders()
{
  return mFolders;
}


//------------------------------------------------------------------------------------//

void SyncConnector::ignoreSslErrors(QNetworkReply *reply)
{
  QList<QSslError> errorsThatCanBeIgnored;
  
  errorsThatCanBeIgnored<<QSslError(QSslError::HostNameMismatch);
  errorsThatCanBeIgnored<<QSslError(QSslError::SelfSignedCertificate);
  reply->ignoreSslErrors(errorsThatCanBeIgnored);
}


//------------------------------------------------------------------------------------//

void SyncConnector::onSslError(QNetworkReply* reply)
{
  reply->ignoreSslErrors();
}


//------------------------------------------------------------------------------------//

bool SyncConnector::checkIfFileExists(QString path)
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

SyncConnector::~SyncConnector()
{
  if (mpSyncProcess != nullptr
      && mpSyncProcess->state() == QProcess::Running)
  {
    mpSyncProcess->kill();
  }
  if (mpSyncthingNotifierProcess != nullptr
      && mpSyncthingNotifierProcess->state() == QProcess::Running)
  {
    mpSyncthingNotifierProcess->kill();
  }
}
  
} // connector
} //mfk
