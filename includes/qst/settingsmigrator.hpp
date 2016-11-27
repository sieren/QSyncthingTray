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

#ifndef settingsmigrator_h
#define settingsmigrator_h
#pragma once
#include <QDateTime>
#include <QSettings>
#include <QString>
#include <QWindow>

#include <qst/identifiers.hpp>
#include <qst/platforms.hpp>

namespace qst
{
namespace settings
{

struct SettingsMigrator
{

public:
SettingsMigrator() :
  mSettings("QSyncthingTray", "qst")
{
  validateSettings();
}

~SettingsMigrator() = default;

//------------------------------------------------------------------------------------//

void validateSettings()
{
  QSettings oldSettings("sieren", "QSyncthingTray");
  oldSettings.setFallbacksEnabled(false);
  mSettings.setFallbacksEnabled(false);
  if (oldSettings.childKeys().size() > 1 && mSettings.childKeys().size() == 0)
  {
    copySettings(mSettings, oldSettings);
  }
  createDefaultSettings();
}


//------------------------------------------------------------------------------------//

void copySettings( QSettings &dst, QSettings &src )
{
  QStringList keys = src.allKeys();
  for( QStringList::iterator i = keys.begin(); i != keys.end(); i++ )
  {
    dst.setValue( *i, src.value( *i ) );
  }
}


//------------------------------------------------------------------------------------//

void createDefaultSettings()
{
  checkAndSetValue(kUrlId, QString("http://127.0.0.1:8384"));
  checkAndSetValue(kMonochromeIconId, false);
  checkAndSetValue(kWebZoomFactorId, 1.0);
  checkAndSetValue(kShutDownExitId, true);
  checkAndSetValue(kNotificationsEnabledId, true);
  checkAndSetValue(kSettingsAvailableId, true);
  checkAndSetValue(kLaunchSyncthingStartupId, false);
  checkAndSetValue(kIconAnimcationsEnabledId, false);
  checkAndSetValue(kPollingIntervalId, 1.0);
  checkAndSetValue(kApiKeyId, qst::utilities::readAPIKey());
  checkAndSetValue(kWebWindowSizeId, QSize(1280, 800));
  checkAndSetValue(kSyncthingPathId,
    sysutils::SystemUtility().getDefaultSyncthingLocation());
  checkAndSetValue(kInotifyPathId,
    sysutils::SystemUtility().getDefaultSyncthingINotifyLocation());
  checkAndSetValue(kLastUpdateCheckId, QDateTime().currentDateTime());
  checkAndSetValue(kLastShownUpdateId, QString("0"));
  checkAndSetValue(kStatsLengthId, 1);
}


//------------------------------------------------------------------------------------//

template <typename T>
void checkAndSetValue(QString key, T value)
{
  if (mSettings.value(key) == QVariant())
  {
    mSettings.setValue(key, value);
  }
}

private:
  QSettings mSettings;
};

} // settings namespace
} // qst namespace
#endif
