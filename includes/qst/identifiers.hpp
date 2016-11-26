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

#ifndef identifiers_h
#define identifiers_h

#include <QString>

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
// Mappings used for application settings

static const QString kUrlId = "url";
static const QString kUserNameId = "username";
static const QString kPasswordId = "userpassword";
static const QString kMonochromeIconId = "monochromeIcon";
static const QString kWebZoomFactorId = "WebZoomFactor";
static const QString kShutDownExitId = "ShutdownOnExit";
static const QString kNotificationsEnabledId = "notificationsEnabled";
static const QString kSettingsAvailableId = "doSettingsExist";
static const QString kLaunchSyncthingStartupId = "launchSyncthingAtStartup";
static const QString kLaunchInotifyStartupId = "launchINotifyAtStartup";
static const QString kIconAnimcationsEnabledId = "animationEnabled";
static const QString kPollingIntervalId = "pollingInterval";
static const QString kApiKeyId = "apiKey";
static const QString kWebWindowSizeId = "WebWindowSize";
static const QString kSyncthingPathId = "syncthingpath";
static const QString kInotifyPathId = "inotifypath";
static const QString kLastUpdateCheckId = "lastupdatecheck";
static const QString kLastShownUpdateId = "lastshownupdatenotification";
static const QString kProcessListId = "processList";
static const QString kStatsLengthId = "statsLength";

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

#endif //identifiers_h
