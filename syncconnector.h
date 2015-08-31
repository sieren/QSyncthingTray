// (c) Matthias Frick 2015

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

using ConnectionStateCallback = std::function<void(std::string, bool)>;
using ConnectionHealthCallback = std::function<void(std::map<std::string, std::string>)>;
using ProcessSpawnedCallback = std::function<void(bool)>;

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
      void spawnSyncThingProcess(std::string filePath);

    private slots:
      void onSslError(QNetworkReply* reply);
      void urlTested(QNetworkReply* reply);
      void connectionHealthReceived(QNetworkReply* reply);
      void checkConnectionHealth();
      void syncThingProcessSpawned(QProcess::ProcessState newState);

    private:
      void ignoreSslErrors(QNetworkReply *reply);
      
      ConnectionStateCallback mConnectionStateCallback = nullptr;
      ConnectionHealthCallback mConnectionHealthCallback = nullptr;
      ProcessSpawnedCallback mProcessSpawnedCallback = nullptr;

      std::thread mIoThread;
      QUrl mCurrentUrl;
      QNetworkAccessManager mWebUrl;
      QNetworkAccessManager mHealthUrl;
      QWebView *mpWebView;
      QProcess *mpSyncProcess;
      std::shared_ptr<QTimer> connectionHealthTimer;
      std::pair<std::string, std::string> mAuthentication;
    };
} // connector
} // mfk



#endif /* defined(__systray__syncconnector__) */
