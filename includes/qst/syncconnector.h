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
#include "apihandler.hpp"
#include <qst/webview.h>

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
} ProcessState;

using ConnectionStateCallback = std::function<void(ConnectionState&)>;
using ProcessStateInfo = std::map<std::string, processState>;

static const std::string kSyncthingIdentifier{"syncthing"};
static const std::string kNotifyIdentifier{"inotify"};

namespace qst
{
namespace connector
{
  class QWebViewClose;
  class SyncConnector : public QObject
  {
    Q_OBJECT
  public:
    explicit SyncConnector(QUrl url, ConnectionStateCallback textCallback);
    virtual ~SyncConnector();
    void setURL(QUrl url, const QString& userName, const QString& password);
    void showWebView();
    void spawnSyncthingProcess(
      std::string filePath,
      const bool shouldSpawn,
      const bool onSetPath = false);
    void checkAndSpawnINotifyProcess(bool isRequestedExternal);
    void shutdownSyncthingProcess();
    std::list<FolderNameFullPath> getFolders();
    LastSyncedFileList getLastSyncedFiles();
    void pauseSyncthing(bool paused);
    void onSettingsChanged();
    webview::WebView *getWebView();

  signals:
    void onConnectionHealthChanged(ConnectionHealthStatus healthState);
    void onProcessSpawned(ProcessStateInfo procState);
    void onNetworkActivityChanged(bool act);

  private slots:
    void onSslError(QNetworkReply* reply);
    void netRequestfinished(QNetworkReply *reply);
    void checkConnectionHealth();
    void syncThingProcessSpawned(QProcess::ProcessState newState);
    void notifyProcessSpawned(QProcess::ProcessState newState);
    void shutdownProcessPosted(QNetworkReply *reply);
    void testUrlAvailability();
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
    void shutdownINotifyProcess();
    int getCurrentVersion(QString reply);
    std::uint16_t mConnectionHealthTime = 1000;
    bool didShowSSLWarning;
    bool mSyncthingPaused = false;
    bool mShouldLaunchINotify = false;

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

    std::unique_ptr<webview::WebView> mpSyncWebView;
    std::unique_ptr<QProcess> mpSyncProcess;
    std::unique_ptr<QProcess> mpSyncthingNotifierProcess;
    std::list<FolderNameFullPath> mFolders;
    LastSyncedFileList mLastSyncedFiles;
    std::unique_ptr<QTimer> mpConnectionHealthTimer;
    std::unique_ptr<QTimer> mpConnectionAvailabilityTimer;
    std::pair<QString, QString> mAuthentication;

    std::string mSyncthingFilePath;
    QString mINotifyFilePath;
    QString mAPIKey;

    qst::sysutils::SystemUtility systemUtil;
    std::unique_ptr<api::APIHandlerBase> mAPIHandler;

    QSettings mSettings;
  };

} // connector
} // qst



#endif /* defined(__systray__syncconnector__) */
