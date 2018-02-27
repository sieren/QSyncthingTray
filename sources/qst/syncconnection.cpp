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

#include <qst/syncconnection.h>
#include <qst/syncconnectionsettings.hpp>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

namespace qst
{
namespace data
{

SyncConnection::SyncConnection(const settings::SyncConnectionSettings& settings,
  QObject* parent) :
    QObject(parent)
  , mConnectionSettings(settings)
  , mpNetwork(new QNetworkAccessManager, &QObject::deleteLater)
{
  mShouldPoll = true;
  configureSSLError();
  mpReconnectTimer = std::unique_ptr<QTimer>(new QTimer());
  connect();
}


void SyncConnection::configureSSLError()
{
  mIgnoreSslErrors.clear();
  mIgnoreSslErrors << QSslError(QSslError::HostNameMismatch);
  mIgnoreSslErrors << QSslError(QSslError::SelfSignedCertificate);
}

void SyncConnection::autoReconnect()
{
  const auto tmp = mReconnectAttempts;
  connect();
  mReconnectAttempts = tmp + 1;
}

void SyncConnection::connect()
{
  mpReconnectTimer->stop();
  mReconnectAttempts = 0;
  if(!isConnected())
  {
    mIsReconnecting = mDidReceiveConfig = mDidReceiveStatus = false;
    if(mConnectionSettings.apiKey.isEmpty() || mConnectionSettings.url.isEmpty())
    {
   //   emit error(tr("Connection configuration is insufficient."), SyncthingErrorCategory::OverallConnection);
      return;
    }
    pollConfig();
    pollStatus();
    mShouldPoll = true;
  }

}

void SyncConnection::pollEvents()
{
  QUrlQuery query;
  if(mLastEventId)
  {
    query.addQueryItem(QStringLiteral("since"), QString::number(mLastEventId));
  }
  mEventsReply = getRequest(QStringLiteral("events"), query);
  QObject::connect(mEventsReply, &QNetworkReply::finished, this, &SyncConnection::parseEvents);
}

void SyncConnection::pollStatus()
{
  auto request = getRequest(QStringLiteral("system/status"), QUrlQuery());
  QObject::connect(request, &QNetworkReply::finished, this, &SyncConnection::parseStatus);
}

void SyncConnection::pollConfig()
{
  auto request = getRequest(tr("system/config"), QUrlQuery());
  QObject::connect(request, &QNetworkReply::finished, this, &SyncConnection::parseConfig);
}

void SyncConnection::parseEvents()
{
  std::cout << "Poll" << std::endl;
  auto* reply = static_cast<QNetworkReply *>(sender());
  reply->deleteLater();

  switch(reply->error())
  {
    case QNetworkReply::NoError:
    {
      QJsonParseError jsonError;
      const QJsonArray replyArr = QJsonDocument::fromJson(reply->readAll(), &jsonError).array();
      for(const QJsonValue &eventVal : replyArr)
      {
        const QJsonObject event = eventVal.toObject();
        mLastEventId = event.value(QStringLiteral("id")).toInt();
        auto evTime = utilities::parse(event.value(QStringLiteral("time")).toString().toStdString());
        const QString eventType(event.value(QStringLiteral("type")).toString());
        const QJsonObject eventData(event.value(QStringLiteral("data")).toObject());
        if(eventType == tr("Starting"))
        {
          onEventStarting(eventData);
        }
        else if(eventType == tr("StateChanged"))
        {
          onEventStateChanged(eventData, evTime);
        }
//        else if(eventType == tr("DownloadProgress"))
//        {
//          onEventDownloadProgress(eventTime, eventData);
//        }
//        else if(eventType.startsWith(tr("Folder")))
//        {
//          onEventFolder(eventTime, eventType, eventData);
//        }
//        else if(eventType.startsWith(tr("Device")))
//        {
//          onEventDevice(eventTime, eventType, eventData);
//        }
//        else if(eventType == tr("ItemStarted"))
//        {
//          onEventItemStarted(eventTime, eventData);
//        }
//        else if(eventType == tr("ItemFinished"))
//        {
//          onEventItemFinished(eventTime, eventData);
//        }
//        else if(eventType == tr("ConfigSaved"))
//        {
//          onEventConfigSaved(); 
//        }
      }
      break;
    }
    case QNetworkReply::TimeoutError:
      // no new events available, keep polling
      break;
    case QNetworkReply::OperationCanceledError:
      // intended disconnect, not an error
      break;
    default:
      break;
  }
  if (mShouldPoll)
  {
    QTimer::singleShot(mConnectionSettings.pollIntervalMs,
      Qt::VeryCoarseTimer, this, &SyncConnection::pollEvents);
  }
}

void SyncConnection::parseStatus()
{
  auto *reply = static_cast<QNetworkReply *>(sender());
  reply->deleteLater();
  
  switch(reply->error())
  {
    case QNetworkReply::NoError:
    {
      QJsonParseError jsonError;
      const QJsonDocument replyDoc = QJsonDocument::fromJson(reply->readAll(), &jsonError);
      if(jsonError.error == QJsonParseError::NoError) {
        const QJsonObject replyObj(replyDoc.object());
        const QString myId(replyObj.value(QStringLiteral("myID")).toString());
        if(myId != mSyncthingInstanceID)
        {
          mSyncthingInstanceID = myId;
     //     emit myIdChanged(mSyncthingInstanceID);
          int index = 0;
//          for(SyncthingDev &dev : m_devs)
//          {
//            if(dev.id == mSyncthingInstanceID)
//            {
//              dev.status = SyncthingDevStatus::OwnDevice;
//              emit devStatusChanged(dev, index);
//              break;
//            }
//            ++index;
//          }
        }
        mDidReceiveStatus = true;
        proceedConnection();
      }
      else
      {
      //  emit error(tr("Unable to parse Syncthing status: ") + jsonError.errorString(), SyncthingErrorCategory::Parsing);
      }
      break;
    }
    case QNetworkReply::OperationCanceledError:
      return;
    default:
      return;
    //  emit error(tr("Unable to request Syncthing status: ") + reply->errorString(), SyncthingErrorCategory::OverallConnection);
  }
}


void SyncConnection::parseConfig()
{
  auto *reply = static_cast<QNetworkReply *>(sender());
  reply->deleteLater();

  switch(reply->error())
  {
    case QNetworkReply::NoError:
    {
      QJsonParseError jsonError;
      const QJsonDocument replyDoc = QJsonDocument::fromJson(reply->readAll(), &jsonError);
      if(jsonError.error == QJsonParseError::NoError)
      {
        const QJsonObject replyObj(replyDoc.object());
//        emit newConfig(replyObj);
//        readDirs(replyObj.value(QStringLiteral("folders")).toArray());
//        readDevs(replyObj.value(QStringLiteral("devices")).toArray());
        mDidReceiveConfig = true;
        if(!mIsConnected)
        {
          proceedConnection();
        }
      }
      else
      {
      //  emit error(tr("Unable to parse Syncthing config: ") + jsonError.errorString(), SyncthingErrorCategory::Parsing);
      }
      break;
    }
    case QNetworkReply::OperationCanceledError:
      return; // intended, not an error
    default:
  //    emit error(tr("Unable to request Syncthing config: ") + reply->errorString(), SyncthingErrorCategory::OverallConnection);
  //    setStatus(SyncthingStatus::Disconnected);
      if(mpReconnectTimer->interval())
      {
        mpReconnectTimer->start();
      }
  }

}

void SyncConnection::proceedConnection()
{
  if(mDidReceiveConfig && mShouldPoll)
  {
    pollConnections();
  //  requestDirStatistics();
  //  requestDeviceStatistics();
  //  requestErrors();
    mLastEventId = 0;
    pollEvents();
  }
}

void SyncConnection::parseConnections()
{
    auto *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    switch(reply->error())
    {
    case QNetworkReply::NoError:
    {
        mIsConnected = true;
        QJsonParseError jsonError;
        const QJsonDocument replyDoc = QJsonDocument::fromJson(reply->readAll(), &jsonError);
        if (jsonError.error == QJsonParseError::NoError)
        {
          const QJsonObject replyObj(replyDoc.object());
          const QJsonObject totalObj(replyObj.value(tr("total")).toObject());

          // read traffic, the conversion to double is neccassary because toInt() doesn't work for high values
          const std::uint64_t totalInTraff =
              static_cast<std::uint64_t>(totalObj.value(tr("inBytesTotal")).toDouble(0.0));
          const std::uint64_t totalOutTraff =
              static_cast<std::uint64_t>(totalObj.value(tr("outBytesTotal")).toDouble(0.0));
          using namespace std::chrono;
          auto now = system_clock::now();
          auto delta = duration_cast<milliseconds>(now - mLastConnectionUpdate);
          if (delta.count() != 0.0)
          {
            mTotalInRate = (totalInTraff - mTotalInTraff) / (delta.count() * 1e-3),
            mTotalOutRate = (totalOutTraff - mTotalOutTraff) / (delta.count() * 1e-3);
            mTotalInRate = std::floor(mTotalInRate * 100) / 100;
            mTotalOutRate = std::floor(mTotalOutRate * 100) / 100;
            mTotalInTraff = totalInTraff;
            mTotalOutTraff = totalOutTraff;
          }
          else
          {
            mTotalInRate = 0.0;
            mTotalOutRate = 0.0;
          }
          emit trafficChanged(mTotalInTraff, mTotalOutTraff);

          // read connection status
          // const QJsonObject connectionsObj(replyObj.value(QStringLiteral("connections")).toObject());
          // int index = 0;
          // for(SyncthingDev &dev : m_devs) {
          //     const QJsonObject connectionObj(connectionsObj.value(dev.id).toObject());
          //     if(!connectionObj.isEmpty()) {
          //         switch(dev.status) {
          //         case SyncthingDevStatus::OwnDevice:
          //             break;
          //         case SyncthingDevStatus::Disconnected:
          //         case SyncthingDevStatus::Unknown:
          //             if(connectionObj.value(QStringLiteral("connected")).toBool(false)) {
          //                 dev.status = SyncthingDevStatus::Idle;
          //             } else {
          //                 dev.status = SyncthingDevStatus::Disconnected;
          //             }
          //             break;
          //         default:
          //             if(!connectionObj.value(QStringLiteral("connected")).toBool(false)) {
          //                 dev.status = SyncthingDevStatus::Disconnected;
          //             }
          //         }
          //         dev.paused = connectionObj.value(QStringLiteral("paused")).toBool(false);
          //         dev.totalIncomingTraffic = static_cast<uint64>(connectionObj.value(QStringLiteral("inBytesTotal")).toDouble(0));
          //         dev.totalOutgoingTraffic = static_cast<uint64>(connectionObj.value(QStringLiteral("outBytesTotal")).toDouble(0));
          //         dev.connectionAddress = connectionObj.value(QStringLiteral("address")).toString();
          //         dev.connectionType = connectionObj.value(QStringLiteral("type")).toString();
          //         dev.clientVersion = connectionObj.value(QStringLiteral("clientVersion")).toString();
          //         emit devStatusChanged(dev, index);
          //     }
          //     ++index;
          // }

          mLastConnectionUpdate = now;

          // since there seems no event for this data, just request every 2 seconds
          if (mShouldPoll)
          {
            QTimer::singleShot(mConnectionSettings.pollIntervalMs,
              Qt::VeryCoarseTimer, this, &SyncConnection::pollConnections);
          }
        }
        else
        {
          //  emit error(tr("Unable to parse connections: ") + jsonError.errorString(), SyncthingErrorCategory::Parsing);
        }
        break;
    }
    case QNetworkReply::OperationCanceledError:
      return; // intended, not an error
    default:
      return;
     //   emit error(tr("Unable to request connections: ") + reply->errorString(), SyncthingErrorCategory::OverallConnection);
    }
}

void SyncConnection::pollConnections()
{
  auto request = getRequest(QStringLiteral("system/connections"), QUrlQuery());
  QObject::connect(request,
    &QNetworkReply::finished, this, &SyncConnection::parseConnections);
}

void SyncConnection::onEventStarting(const QJsonObject& evt)
{
  auto idVal = evt.value(tr("myID")).toString();
  auto confVal = evt.value(tr("home")).toString();
  if(idVal != mSyncthingInstanceID || confVal != mSyncthingConfigPath)
  {
    mSyncthingInstanceID = idVal;
    mSyncthingConfigPath = confVal;
    emit configPathChanged(confVal);
  }
}


void SyncConnection::onEventStateChanged(const QJsonObject& evt, utilities::DateTimePoint time)
{
  std::cout << "Event Time: " << time.time_since_epoch().count() << std::endl;
}


QNetworkRequest SyncConnection::buildRequest(const QString& urlPath, const QUrlQuery& query, const bool restApi)
{
  QUrl url(mConnectionSettings.url);
  url.setQuery(query);
  url.setPath(restApi ? (url.path() + QStringLiteral("/rest/") + urlPath) : (url.path() + urlPath));
  url.setUserName(mConnectionSettings.username);
  url.setPassword(mConnectionSettings.password);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
  request.setRawHeader("X-API-Key", mConnectionSettings.apiKey);
  return request;
}

QNetworkReply* SyncConnection::getRequest(const QString& urlPath, const QUrlQuery& query, const bool restApi)
{
  auto *reply = mpNetwork->get(buildRequest(urlPath, query, restApi));
  reply->ignoreSslErrors(mIgnoreSslErrors);
  return reply;
}

QNetworkReply* SyncConnection::postRequest(const QString& urlPath, const QUrlQuery& query, const QByteArray &data)
{
  auto *reply = mpNetwork->post(buildRequest(urlPath, query), data);
  reply->ignoreSslErrors(mIgnoreSslErrors);
  return reply;
}

std::uint64_t SyncConnection::totalInTraff() const
{
  return mTotalInTraff;
}
std::uint64_t SyncConnection::totalOutTraff() const
{
  return mTotalOutTraff;
}
std::uint64_t SyncConnection::totalInRate() const
{
  return mTotalInRate;
}
std::uint64_t SyncConnection::totalOutRate() const
{
  return mTotalOutRate;
}
bool SyncConnection::isConnected() const
{
  return mIsConnected;
}

const std::vector<model::SyncDevice> &SyncConnection::devices() const
{
  return mDevices;
}
  
const std::vector<data::SyncthingDirectory> &SyncConnection::directories() const
{
  return mDirectories;
}

} // namespace connector
} // namespace qst
