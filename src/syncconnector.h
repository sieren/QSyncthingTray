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

#ifndef __systray__syncconnector__
#define __systray__syncconnector__

#pragma once
#include <stdio.h>
#include <QObject>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QProcess>
#include <memory>
#include <functional>
#include <map>
#include <thread>
#include <utility>
#include "platforms.hpp"
#include "syncwebview.h"
#include "apihandler.hpp"

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
QT_END_NAMESPACE

typedef enum processState
{
  SPAWNED,
  NOT_RUNNING,
  ALREADY_RUNNING,
  PAUSED
} kSyncthingProcessState;

using ConnectionStateCallback = std::function<void(std::pair<std::string, bool>)>;
using ConnectionHealthCallback = std::function<void(ConnectionHealthStatus)>;
using NetworkActivityCallback = std::function<void(bool)>;
using ProcessSpawnedCallback = std::function<void(kSyncthingProcessState)>;

namespace mfk
{
namespace connector
{
  class QWebViewClose;
  class SyncConnector : public QObject
  {
    Q_OBJECT
  public:
    explicit SyncConnector(QUrl url);
    virtual ~SyncConnector();
    void setURL(QUrl url, std::string userName, std::string password,
      ConnectionStateCallback setText);
    void setConnectionHealthCallback(ConnectionHealthCallback cb);
    void setProcessSpawnedCallback(ProcessSpawnedCallback cb);
    void setNetworkActivityCallback(NetworkActivityCallback cb);
    void showWebView();
    void spawnSyncthingProcess(
      std::string filePath,
      const bool shouldSpawn,
      const bool onSetPath = false);
    void spawnINotifyProcess(
     std::string filePath,
     const bool shouldSpawn,
     const bool onSetPath = false);
    void shutdownSyncthingProcess();
    std::list<FolderNameFullPath> getFolders();
    LastSyncedFileList getLastSyncedFiles();

  private slots:
    void onSslError(QNetworkReply* reply);
    void netRequestfinished(QNetworkReply *reply);
    void checkConnectionHealth();
    void syncThingProcessSpawned(QProcess::ProcessState newState);
    void shutdownProcessPosted(QNetworkReply *reply);

  private:
    void ignoreSslErrors(QNetworkReply *reply);
    void getCurrentConfig();
    bool checkIfFileExists(QString path);
    void urlTested(QNetworkReply* reply);
    void connectionHealthReceived(QNetworkReply* reply);
    void currentConfigReceived(QNetworkReply* reply);
    void lastSyncedFilesReceived(QNetworkReply *reply);
    void killProcesses();
    int getCurrentVersion(std::string reply);

    template <typename T>
    std::string trafficToString(T traffic);

    ConnectionStateCallback mConnectionStateCallback = nullptr;
    ConnectionHealthCallback mConnectionHealthCallback = nullptr;
    ProcessSpawnedCallback mProcessSpawnedCallback = nullptr;
    NetworkActivityCallback mNetworkActivityCallback = nullptr;
    std::thread mIoThread;
    QUrl mCurrentUrl;

    //! Network access, new methods should be added here
    //! so the called function can dispatch accordingly
    QNetworkAccessManager network;
    enum class kRequestMethod {
      urlTested,
      connectionHealth,
      getCurrentConfig,
      getLastSyncedFiles,
    };
    QHash<QNetworkReply*, kRequestMethod> requestMap;

    std::unique_ptr<SyncWebView> mpSyncWebView;
    std::unique_ptr<QProcess> mpSyncProcess;
    std::unique_ptr<QProcess> mpSyncthingNotifierProcess;
    std::list<FolderNameFullPath> mFolders;
    LastSyncedFileList mLastSyncedFiles;
    std::unique_ptr<QTimer> mpConnectionHealthTimer;
    std::pair<std::string, std::string> mAuthentication;
    std::shared_ptr<SyncConnector> mpSyncConnector;
    
    std::string mAPIKey;
    
    mfk::sysutils::SystemUtility systemUtil;
    std::unique_ptr<api::APIHandlerBase> mAPIHandler;
  };

} // connector
} // mfk



#endif /* defined(__systray__syncconnector__) */
