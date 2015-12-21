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
    
    virtual std::map<std::string, std::string> getConnections(QByteArray reply) = 0;
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
  };
  
  // API Specializations
  
  // Syncthing API V11 Specializations
  struct V12API : public APIHandlerBase
  {
    std::map<std::string, std::string> getConnections(QByteArray reply) override
    {
      std::map<std::string, std::string> result;
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
    std::map<std::string, std::string> getConnections(QByteArray reply) override
    {
      std::map<std::string, std::string> result;
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
