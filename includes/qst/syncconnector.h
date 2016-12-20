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
#include <cstdint>
#include <functional>
#include <map>
#include <thread>
#include <utility>
#include "platforms.hpp"
#include "apihandler.hpp"
#include <qst/appsettings.hpp>
#include <qst/webview.h>


using ConnectionStateData = std::pair<ConnectionHealthData, TrafficData>;
using ConnectionStateCallback = std::function<void(ConnectionState&)>;


namespace qst
{
namespace connector
{
  class QWebViewClose;
  class SyncConnector : public QObject
  {
    Q_OBJECT
  public:
    explicit SyncConnector(QUrl url, ConnectionStateCallback textCallback,
      std::shared_ptr<settings::AppSettings> appSettings);
    virtual ~SyncConnector();
    void setURL(QUrl url, const QString& userName, const QString& password);
    void showWebView();
    void shutdownSyncthingProcess();
    std::list<FolderNameFullPath> getFolders();
    LastSyncedFileList getLastSyncedFiles();
    void pauseSyncthing(bool paused);
    webview::WebView *getWebView();

  signals:
    void onConnectionHealthChanged(ConnectionStateData healthState);
    void onNetworkActivityChanged(bool act);

  private slots:
    void onSslError(QNetworkReply* reply);
    void netRequestfinished(QNetworkReply *reply);
    void checkConnectionHealth();
    void shutdownProcessPosted(QNetworkReply *reply);
    void testUrlAvailability();
    void webViewClosed();
    void onSettingsChanged();

  private:
    void ignoreSslErrors(QNetworkReply *reply);
    void getCurrentConfig();
    void resetNetworkAccessManager();
    bool checkIfFileExists(QString path);
    void urlTested(QNetworkReply* reply);
    void connectionHealthReceived(QNetworkReply* reply);
    void currentConfigReceived(QNetworkReply* reply);
    void lastSyncedFilesReceived(QNetworkReply *reply);
    int getCurrentVersion(QString reply);
    std::uint16_t mConnectionHealthTime = 1000;
    bool didShowSSLWarning;
    bool mSyncthingPaused = false;
    bool mShouldLaunchINotify = false;

    ConnectionStateCallback mConnectionStateCallback = nullptr;
    QUrl mCurrentUrl;

    //! Network access, new methods should be added here
    //! so the called function can dispatch accordingly
    QSharedPointer<QNetworkAccessManager> mpNetwork;
    enum class kRequestMethod {
      urlTested,
      connectionHealth,
      getCurrentConfig,
      getLastSyncedFiles,
      shutdownRequested
    };
    QHash<QNetworkReply*, kRequestMethod> requestMap;

    std::unique_ptr<webview::WebView> mpSyncWebView;
    std::list<FolderNameFullPath> mFolders;
    LastSyncedFileList mLastSyncedFiles;
    std::unique_ptr<QTimer> mpConnectionHealthTimer;
    std::unique_ptr<QTimer> mpConnectionAvailabilityTimer;
    std::pair<QString, QString> mAuthentication;

    QString mSyncthingFilePath;
    QString mINotifyFilePath;
    QString mAPIKey;

    std::unique_ptr<api::APIHandlerBase> mAPIHandler;
    std::shared_ptr<settings::AppSettings> mpAppSettings;
  };

} // connector
} // qst



#endif /* defined(__systray__syncconnector__) */
