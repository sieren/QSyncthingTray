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

#ifndef apihandler_h
#define apihandler_h
#pragma once
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <cmath>
#include <map>
#include <chrono>
#include <algorithm>
#include <vector>
#include <limits>
#include <tuple>
#include "utilities.hpp"

#define kInternalChangedFilesCache 5

namespace
{
  using DateFolderFile = std::tuple<QString, QString, QString, bool>;
  using LastSyncedFileList = std::vector<DateFolderFile>;
  using ConnectionHealthData = std::map<QString, QVariant>;
  using FolderNameFullPath = std::pair<QString, QString>;
  using ConnectionState = std::pair<QString, bool>;
  using TrafficData = std::tuple<double, double, std::chrono::time_point<std::chrono::system_clock>>;
} // anon

namespace qst
{
namespace api
{

  struct APIHandlerBase
  {
    const int version = 0;

    APIHandlerBase() = default;

    virtual ConnectionHealthData getConnections(QByteArray reply) = 0;

    APIHandlerBase *getAPIForVersion(int version);

    // Consistent across V11/V12

    auto getCurrentFolderList(QByteArray reply) -> std::list<FolderNameFullPath>
    {
      std::list<FolderNameFullPath> result;

      if (reply.size() > 0)
      {
        QString m_DownloadedData = static_cast<QString>(reply);
        QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
        QJsonObject replyData = replyDoc.object();
        QJsonArray foldersArray = replyData["folders"].toArray();
        QJsonArray::iterator it;
        foreach (const QJsonValue & value, foldersArray)
        {
          FolderNameFullPath aResult;
          QJsonObject singleEntry = value.toObject();
          aResult.first = singleEntry.find("id").value().toString();
          aResult.second = singleEntry.find("path").value().toString();
          result.emplace_back(aResult);
        }
      }
      return result;
    }

    // return current traffic in byte/s
    auto getCurrentTraffic(QByteArray reply) -> TrafficData
    {
      using namespace std::chrono;
      auto now = system_clock::now();
      auto timeDelta = duration_cast<milliseconds>(now - std::get<2>(oldTraffic));
      double curInBytes, curOutBytes;
      if (reply.size() == 0)
      {
        curInBytes = curOutBytes = (std::numeric_limits<double>::min)();
      }
      else
      {
        QString m_DownloadedData = static_cast<QString>(reply);
        QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
        QJsonObject replyData = replyDoc.object();
        QJsonObject connectionArray = replyData["total"].toObject();
        double inBytes = static_cast<double>(connectionArray.find("inBytesTotal").value().toDouble());
        double outBytes = static_cast<double>(connectionArray.find("outBytesTotal").value().toDouble());
        curInBytes = (std::max)(0.0, ((inBytes - std::get<0>(oldTraffic)) / (timeDelta.count() * 1e-3)))
          + (std::numeric_limits<double>::min)();
        curOutBytes = (std::max)(0.0, ((outBytes - std::get<1>(oldTraffic)) / (timeDelta.count()* 1e-3)))
          + (std::numeric_limits<double>::min)();
        curInBytes = std::floor(curInBytes * 100) / 100;
        curOutBytes = std::floor(curOutBytes * 100) / 100;
        oldTraffic = std::make_tuple(inBytes, outBytes, now);
      }
      return std::make_tuple(std::move(curInBytes/kBytesToKilobytes),
        std::move(curOutBytes/kBytesToKilobytes), std::move(now));
    }

    auto getLastSyncedFiles(QByteArray reply) -> LastSyncedFileList
    {
      QString m_DownloadedData = static_cast<QString>(reply);
      QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
      QJsonObject replyData = replyDoc.object();

      QStringList folderNames = replyData.keys();
      for (QString folderName : folderNames)
      {
        QJsonObject mainPathInfo = replyData[folderName].toObject();
        QJsonObject fileInfo = mainPathInfo["lastFile"].toObject();

        QString folderNameStr = folderName;
        QString lastDate = fileInfo.find("at").value().toString();
        QString fileName = fileInfo.find("filename").value().toString();
        bool isDeleted = fileInfo.find("deleted").value().toBool();

        fileList.erase(std::remove_if(fileList.begin(), fileList.end(),
          [&](const DateFolderFile &item)
          {
            return std::get<2>(item) == fileName;
          }), fileList.end());
        DateFolderFile item = std::make_tuple(
          lastDate, folderNameStr, fileName, isDeleted);
        if (fileName != "")
        {
          fileList.emplace_back(item);
        }
      }

      std::sort(fileList.begin(), fileList.end(), [](
        const DateFolderFile &rhs, const DateFolderFile &lhs)
        {
          return std::get<0>(rhs) > std::get<0>(lhs);
        });

      if (fileList.size() < kInternalChangedFilesCache)
      {
        fileList.shrink_to_fit();
      }
      else
      {
        fileList.resize(kInternalChangedFilesCache);
      }
      return fileList;
    }

    std::tuple<float, float, std::chrono::time_point<std::chrono::system_clock>> oldTraffic;
    LastSyncedFileList fileList;
  };

  // API Specializations

  template<const int Version>
  struct API;

  // Syncthing API V11 Specializations
  template<>
  struct API<11> : public APIHandlerBase
  {
    const int version = 11;

    auto getConnections(QByteArray reply) -> ConnectionHealthData override
    {
      ConnectionHealthData result;
      result.emplace("state", "0");
      if (reply.size() == 0)
      {
        result.emplace("activeConnections", 0);
        result.emplace("totalConnections", 0);
      }
      else
      {
        result.clear();
        result.emplace("state", 1);
        QString m_DownloadedData = static_cast<QString>(reply);
        QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
        QJsonObject replyData = replyDoc.object();
        QJsonObject connectionArray = replyData["connections"].toObject();
        result.emplace("activeConnections", connectionArray.size());
        result.emplace("totalConnections", connectionArray.size());
      }
      return result;
    }
  };

  // Syncthing API V12 Specializations
  template<>
  struct API<12> : public APIHandlerBase
  {
    const int version = 12;

    auto getConnections(QByteArray reply) -> ConnectionHealthData override
    {
      ConnectionHealthData result;
      result.emplace("state", "0");
      if (reply.size() == 0)
      {
        result.emplace("activeConnections", 0);
        result.emplace("totalConnections", 0);
      }
      else
      {
        result.clear();
        result.emplace("state", 1);
        QString m_DownloadedData = static_cast<QString>(reply);
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
        result.emplace("activeConnections", active);
        result.emplace("totalConnections", connectionArray.size());
      }
      return result;
    }
  };

  // Syncthing API V13 Specializations
  template<>
  struct API<13> : public API<12>
  {
    const int version = 13;
    using API<12>::getConnections;
  };


  template<typename NetReply>
  struct APIHandlerFactory
  {
    inline auto getAPIForVersion(int version) -> APIHandlerBase*
    {
      switch (version)
      {
        case 13:
          return new API<13>;
          break;
        case 12:
          return new API<12>;
          break;
        case 11:
          return new API<11>;
          break;
        default:
          return new API<13>;
      }
    }


    auto getConnectionVersionInfo(NetReply *reply) -> std::pair<QString, bool>
    {
      QString result;
      bool success = false;
      
      if (reply->error() != NetReply::NoError)
      {
        result = reply->errorString();
      }
      else
      {
        QString m_DownloadedData = static_cast<QString>(reply->readAll());
        QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
        QJsonObject replyData = replyDoc.object();
        result = replyData.value("version").toString();
        success = true;
      }
      return {result, success};
    }
  };

} // api
} // qst
#endif /* apihandler_h */

