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
#include <QSettings>
#include <memory>
#include <cstdint>
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

using ConnectionStateCallback = std::function<void(ConnectionState)>;

namespace qst
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
    void pauseSyncthing(bool paused);
    void onSettingsChanged();
    SyncWebView *getWebView();

  signals:
    void onConnectionHealthChanged(ConnectionHealthStatus healthState);
    void onProcessSpawned(kSyncthingProcessState procState);
    void onNetworkActivityChanged(bool act);

  private slots:
    void onSslError(QNetworkReply* reply);
    void netRequestfinished(QNetworkReply *reply);
    void checkConnectionHealth();
    void syncThingProcessSpawned(QProcess::ProcessState newState);
    void shutdownProcessPosted(QNetworkReply *reply);
    void webViewClosed();

  private:
    void ignoreSslErrors(QNetworkReply *reply);
    void getCurrentConfig();
    bool checkIfFileExists(QString path);
    void urlTested(QNetworkReply* reply);
    void connectionHealthReceived(QNetworkReply* reply);
    void currentConfigReceived(QNetworkReply* reply);
    void lastSyncedFilesReceived(QNetworkReply *reply);
    void killProcesses();
    int getCurrentVersion(QString reply);
    std::uint16_t mConnectionHealthTime = 1000;
    bool didShowSSLWarning;
    bool mSyncthingPaused = false;

    template <typename T>
    QString trafficToString(T traffic);

    ConnectionStateCallback mConnectionStateCallback = nullptr;
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
      shutdownRequested
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

    std::string mSyncthingFilePath;
    std::string mINotifyFilePath;
    QString mAPIKey;

    qst::sysutils::SystemUtility systemUtil;
    std::unique_ptr<api::APIHandlerBase> mAPIHandler;

    QSettings mSettings;
  };

} // connector
} // qst



#endif /* defined(__systray__syncconnector__) */
