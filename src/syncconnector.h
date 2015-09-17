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
#include <QWebView>
#include <QProcess>
#include <memory>
#include <functional>
#include <map>
#include <thread>
#include <utility>
#include "systemUtils.hpp"
#if defined(__APPLE__) && defined(__MACH__) || defined(__linux__)
/* Apple OSX and iOS (Darwin) */
#include "posixUtils.hpp"
#endif
#ifdef _WIN32
#include "winUtils.hpp"
#endif

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
  ALREADY_RUNNING
} kSyncthingProcessState;

using ConnectionStateCallback = std::function<void(std::string, bool)>;
using ConnectionHealthCallback = std::function<void(std::map<std::string, std::string>)>;
using ProcessSpawnedCallback = std::function<void(kSyncthingProcessState)>;

namespace mfk
{
namespace connector
{
  class SyncConnector : public QObject
    {
      Q_OBJECT
    public:
      //  explicit SyncConnector() = default;
      explicit SyncConnector(QUrl url);
      virtual ~SyncConnector();
      void setURL(QUrl url, std::string userName, std::string password, ConnectionStateCallback setText);
      void setConnectionStateCallback();
      void setConnectionHealthCallback(ConnectionHealthCallback cb);
      void setProcessSpawnedCallback(ProcessSpawnedCallback cb);
      void showWebView();
      void spawnSyncthingProcess(std::string filePath);
      std::list<std::pair<std::string, std::string>> getFolders();

    private slots:
      void onSslError(QNetworkReply* reply);
      void urlTested(QNetworkReply* reply);
      void connectionHealthReceived(QNetworkReply* reply);
      void folderListReceived(QNetworkReply* reply);
      void checkConnectionHealth();
      void syncThingProcessSpawned(QProcess::ProcessState newState);

    private:
      void ignoreSslErrors(QNetworkReply *reply);
      void checkFolderList();
      bool checkIfFileExists(QString path);
      ConnectionStateCallback mConnectionStateCallback = nullptr;
      ConnectionHealthCallback mConnectionHealthCallback = nullptr;
      ProcessSpawnedCallback mProcessSpawnedCallback = nullptr;
      std::thread mIoThread;
      QUrl mCurrentUrl;
      QNetworkAccessManager mWebUrl;
      QNetworkAccessManager mHealthUrl;
      QNetworkAccessManager mFolderUrl;
      std::unique_ptr<QWebView> mpWebView;
      QProcess *mpSyncProcess;
      std::list<std::pair<std::string, std::string>> mFolders;
      std::shared_ptr<QTimer> connectionHealthTimer;
      std::pair<std::string, std::string> mAuthentication;
      #if (defined(__APPLE__) && defined(__MACH__)) || defined(__linux__)
            /* Apple OSX and iOS (Darwin) */
      mfk::sysutils::SystemUtility<sysutils::PosixUtils> systemUtil;
      #endif
	    #ifdef _WIN32
      mfk::sysutils::SystemUtility<sysutils::WinUtils> systemUtil;
      #endif
    };
} // connector
} // mfk



#endif /* defined(__systray__syncconnector__) */
