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

#ifndef SYNCCONNECTION_H
#define SYNCCONNECTION_H

#include <QWidget>
#include <QWidgetAction>
#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QTimer>
#include <QUrl>
#include <qst/appsettings.hpp>
#include <qst/syncdevice.hpp>
#include <qst/syncconnectionsettings.hpp>
#include <memory>

class QNetworkRequest;
class QNetworkReply;

namespace qst
{
namespace connector
{

enum class ConnectionStatus {
  Connected,
  Disconnected,
  Reconnecting,
  Idle,
  Scanning,
  Paused,
  Syncing,
  OutOfSync,
  Destruction
};

class SyncConnection : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString configPath READ configPath NOTIFY configPathChanged)
  Q_PROPERTY(double totalInTraff READ totalInTraff NOTIFY trafficChanged)
  Q_PROPERTY(double totalOutTraff READ totalOutTraff NOTIFY trafficChanged)
  Q_PROPERTY(double totalInRate READ totalInRate NOTIFY trafficChanged)
  Q_PROPERTY(double totalOutRate READ totalOutRate NOTIFY trafficChanged)
signals:
  void configPathChanged(const QString& newPath);
  void trafficChanged(const std::uint64_t mInTraff, const std::uint64_t mOutTraff);
  void onConfigChanged(const QJsonObject& config);
  // void onDirectoriesChanged(const std::vector<SyncthingDir>& dirs);
  void onDevicesChanged(const std::vector<model::SyncDevice>& devices);

public:
  SyncConnection() = default;
  SyncConnection(const settings::SyncConnectionSettings& settings, QObject* parent = nullptr);
  ~SyncConnection() = default;
  SyncConnection(const SyncConnection&) = delete;
  SyncConnection& operator=(const SyncConnection&) = delete;
  SyncConnection(SyncConnection&& rhs) = default;
  SyncConnection& operator=(SyncConnection&&) noexcept { return *this; };

  inline const QString& configPath() const
  {
    return mSyncthingConfigPath;
  };

  void autoReconnect();

  std::uint64_t totalInTraff() const;
  std::uint64_t totalOutTraff() const;
  std::uint64_t totalInRate() const;
  std::uint64_t totalOutRate() const;
  const std::vector<model::SyncDevice> &devices() const;
  bool isConnected() const;

private:
  settings::SyncConnectionSettings mConnectionSettings;
  std::vector<model::SyncDevice> mDevices;

  //! Network access, new methods should be added here
  //! so the called function can dispatch accordingly
  QSharedPointer<QNetworkAccessManager> mpNetwork;
  QNetworkRequest buildRequest(const QString& urlPath, const QUrlQuery& query, const bool restApi = true);
  QNetworkReply* getRequest(const QString& urlPath, const QUrlQuery& query, const bool restApi = true);
  QNetworkReply* postRequest(const QString& urlPath, const QUrlQuery& query, const QByteArray &data);

  QList<QSslError> mIgnoreSslErrors;
  void configureSSLError();

  QNetworkReply *mEventsReply;
  //! Network events
  bool mShouldPoll = false;
  int mLastEventId = 0;

public slots:
  void parseEvents();
  void parseConnections();
  void parseConfig();
  void parseStatus();
  void pollEvents();
private:
  void connect();
  void pollConnections();
  void pollStatus();
  void pollConfig();
  void proceedConnection();
  //! Event Parsing
  void onEventStarting(const QJsonObject& evt);
  void onEventStateChanged(const QJsonObject& evt, utilities::DateTimePoint time);
  //! Internal State Representation of Syncthing
  //! Should be in sync with REST API
  QString mSyncthingInstanceID;

  bool mIsConnected = false;
  bool mIsReconnecting = false;
  int mReconnectAttempts = 0;
  
  bool mDidReceiveConfig = false;
  bool mDidReceiveStatus = false;
  QString mSyncthingConfigPath;
  std::uint64_t mTotalInTraff;
  std::uint64_t mTotalOutTraff;
  std::uint64_t mTotalInRate;
  std::uint64_t mTotalOutRate;
  std::chrono::time_point<std::chrono::system_clock> mLastConnectionUpdate;
  

  std::unique_ptr<QTimer> mpNetworkPollTimer;
  std::unique_ptr<QTimer> mpReconnectTimer;
};
}
}
#endif
