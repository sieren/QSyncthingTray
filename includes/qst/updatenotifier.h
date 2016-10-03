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

#ifndef UPDATENOTIFIER_H
#define UPDATENOTIFIER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <functional>

//------------------------------------------------------------------------------------//

static const QString kGithubUrl =
  "https://api.github.com/repos/sieren/QSyncthingTray/releases/latest";
static const double kUpdateInterval = 604800000; // 7 days
static const QString kTagKey = "tag_name";

//------------------------------------------------------------------------------------//
using NotificationCallback = std::function<void(bool)>;
//------------------------------------------------------------------------------------//

namespace qst
{
namespace update
{
  class UpdateNotifier : QObject
{
  Q_OBJECT
public:
  UpdateNotifier(NotificationCallback callback, const QString& currentVer);
  ~UpdateNotifier() = default;
  UpdateNotifier(UpdateNotifier&&) = delete;
  UpdateNotifier(const UpdateNotifier&) = delete;
  UpdateNotifier& operator=(const UpdateNotifier&) = delete;

public slots:
  void checkUpdate(const bool manualCheck = false);

private slots:
  void netRequestFinished(QNetworkReply* reply);

private:
  NotificationCallback mNotifyCallback;
  bool mDidCheckManually = false;
  void performNetRequest();
  QSettings mSettings;
  QString mCurrentVer;

  QTimer mUpdateCheckTimer;
  QNetworkAccessManager network;
};

} // update namespace
} // qst namespace

#endif
