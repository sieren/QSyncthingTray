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


#include <qst/updatenotifier.h>
#include <QContextMenuEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <functional>
#include <iostream>

//------------------------------------------------------------------------------------//

namespace qst
{
namespace update
{

//------------------------------------------------------------------------------------//

UpdateNotifier::UpdateNotifier(NotificationCallback callback,
  const QString& currentVer) :
    mNotifyCallback(callback)
  , mSettings("QSyncthingTray", "qst")
  , mCurrentVer(currentVer)
{
  connect(
    &network, SIGNAL (finished(QNetworkReply*)),
    this, SLOT (netRequestFinished(QNetworkReply*))
    );
  connect(&mUpdateCheckTimer, SIGNAL(timeout()), this,
    SLOT(checkUpdate()));
  mUpdateCheckTimer.start(30000);
}

//------------------------------------------------------------------------------------//

void UpdateNotifier::checkUpdate(const bool manualCheck)
{
  mDidCheckManually = manualCheck;
  if (!mDidCheckManually)
  {
    auto lastUpdate = mSettings.value("lastupdatecheck").toDateTime();
    auto timeDiff = lastUpdate.msecsTo(QDateTime().currentDateTime());
    if (timeDiff > kUpdateInterval)
    {
      performNetRequest();
    }
  }
  else
  {
    performNetRequest();
  }
}


//------------------------------------------------------------------------------------//

void UpdateNotifier::performNetRequest()
{
  QUrl url(kGithubUrl);
  QNetworkRequest request(url);
  network.clearAccessCache();
  network.get(request);
  mSettings.setValue("lastupdatecheck", QDateTime().currentDateTime());
}


//------------------------------------------------------------------------------------//

void UpdateNotifier::netRequestFinished(QNetworkReply* reply)
{
  QByteArray replyData;
  if (reply->error() == QNetworkReply::NoError)
  {
    replyData = reply->readAll();
  }
  else
  {
    return;
  }
  QString data = static_cast<QString>(replyData);
  QJsonDocument replyDoc = QJsonDocument::fromJson(data.toUtf8());
  QJsonObject replyObj = replyDoc.object();

  auto lastKey = replyObj.find(kTagKey).value().toString();
  auto lastShownForVersion = mSettings.value("lastshownupdatenotification").toString();
  if (lastKey != mCurrentVer && lastKey != lastShownForVersion)
  {
    mNotifyCallback(true);
    mSettings.setValue("lastshownupdatenotification", lastKey);
    return;
  }
  else if (mDidCheckManually)
  {
    mNotifyCallback(!(lastKey == mCurrentVer));
  }
}


//------------------------------------------------------------------------------------//

} // update namespace
} // qst namespace
