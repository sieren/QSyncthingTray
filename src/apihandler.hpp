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
#include <map>
#include <chrono>
#include <algorithm>
#include <vector>
#include <limits>
#include "utilities.hpp"

#define kInternalChangedFilesCache 5

namespace
{
  using DateFolderFile = std::tuple<std::string, std::string, std::string, bool>;
  using LastSyncedFileList = std::vector<DateFolderFile>;
  using ConnectionHealthStatus = std::map<std::string, std::string>;
} // anon

namespace mfk
{
namespace api
{
  struct V12API;

  struct APIHandlerBase
  {
    std::pair<std::string, bool> getConnectionInfo(QNetworkReply *reply)
    {
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
        result = replyData.value("version").toString().toStdString();
        success = true;
      }
      return {result, success};
    }
    
    virtual ConnectionHealthStatus getConnections(QByteArray reply) = 0;

    APIHandlerBase *getAPIForVersion(int version);
    
    // Consistent across V11/V12
    
    std::list<std::pair<std::string, std::string>> getCurrentFolderList(QByteArray reply)
    {
      std::list<std::pair<std::string, std::string>> result;

      if (reply.size() > 0)
      {
        QString m_DownloadedData = static_cast<QString>(reply);
        QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
        QJsonObject replyData = replyDoc.object();
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
      }
      return result;
    }
    
    // return current traffic in byte/s
    std::pair<double, double> getCurrentTraffic(QByteArray reply)
    {
      using namespace std::chrono;
      auto now = system_clock::now();
      auto timeDelta = duration_cast<milliseconds>(now - std::get<2>(oldTraffic));
      float curInBytes, curOutBytes;
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
        float inBytes = static_cast<float>(connectionArray.find("inBytesTotal").value().toDouble());
        float outBytes = static_cast<float>(connectionArray.find("outBytesTotal").value().toDouble());
        curInBytes = (std::max)(0.0, ((inBytes - std::get<0>(oldTraffic)) / (timeDelta.count() * 1e-3)))
          + (std::numeric_limits<double>::min)();
        curOutBytes = (std::max)(0.0, ((outBytes - std::get<1>(oldTraffic)) / (timeDelta.count()* 1e-3)))
          + (std::numeric_limits<double>::min)();
        oldTraffic = std::make_tuple(inBytes, outBytes, now);
      }
      return {curInBytes/kBytesToKilobytes, curOutBytes/kBytesToKilobytes};
    }
    
    std::string getCurrentAPIKey(QByteArray reply)
    {
      std::string apiKey;
      if (reply.size() > 0)
      {
        QString m_DownloadedData = static_cast<QString>(reply);
        QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
        QJsonObject replyData = replyDoc.object();
        QJsonObject guiData = replyData["gui"].toObject();
        apiKey =  guiData["apiKey"].toString().toStdString();
      }
      return apiKey;
    }
    
    LastSyncedFileList getLastSyncedFiles(QByteArray reply)
    {
      QString m_DownloadedData = static_cast<QString>(reply);
      QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
      QJsonObject replyData = replyDoc.object();

      QStringList folderNames = replyData.keys();
      for (QString folderName : folderNames)
      {
        QJsonObject mainPathInfo = replyData[folderName].toObject();
        QJsonObject fileInfo = mainPathInfo["lastFile"].toObject();

        std::string folderNameStr = folderName.toStdString();
        std::string lastDate = fileInfo.find("at").value().toString().toStdString();
        std::string fileName = fileInfo.find("filename").value().toString().toStdString();
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
  
  // Syncthing API V11 Specializations
  struct V12API : public APIHandlerBase
  {
    ConnectionHealthStatus getConnections(QByteArray reply) override
    {
      ConnectionHealthStatus result;
      result.emplace("state", "0");
      if (reply.size() == 0)
      {
        result.emplace("activeConnections", std::to_string(0));
        result.emplace("totalConnections", std::to_string(0));
      }
      else
      {
        result.clear();
        result.emplace("state", "1");
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
        result.emplace("activeConnections", std::to_string(active));
        result.emplace("totalConnections", std::to_string(connectionArray.size()));
      }
      return result;
    }
  };
  
  // Syncthing API V11 Specializations
  struct V11API : public APIHandlerBase
  {
    ConnectionHealthStatus getConnections(QByteArray reply) override
    {
      ConnectionHealthStatus result;
      result.emplace("state", "0");
      if (reply.size() == 0)
      {
        result.emplace("activeConnections", std::to_string(0));
        result.emplace("totalConnections", std::to_string(0));
      }
      else
      {
        result.clear();
        result.emplace("state", "1");
        QString m_DownloadedData = static_cast<QString>(reply);
        QJsonDocument replyDoc = QJsonDocument::fromJson(m_DownloadedData.toUtf8());
        QJsonObject replyData = replyDoc.object();
        QJsonObject connectionArray = replyData["connections"].toObject();
        result.emplace("activeConnections", std::to_string(connectionArray.size()));
        result.emplace("totalConnections", std::to_string(connectionArray.size()));
      }
      return result;
    }
  };
  
  inline APIHandlerBase *APIHandlerBase::getAPIForVersion(int version)
  {
    switch (version)
    {
      case 12:
        return new V12API;
        break;
      case 11:
        return new V11API;
        break;
      default:
        return new V12API;
    }
  }
  
} // api
} // mfk
#endif /* apihandler_h */
