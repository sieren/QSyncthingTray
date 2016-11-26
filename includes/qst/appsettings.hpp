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

#ifndef appsettings_h
#define appsettings_h
#pragma once
#include <QObject>
#include <QSettings>
#include <QString>
#include <tuple>
#include "utilities.hpp"
#include <qst/identifiers.hpp>
#include <qst/settingsmigrator.hpp>


namespace qst
{
namespace settings
{
class AppSettings : public QObject
{
  Q_OBJECT
public:
  AppSettings() :
  mSettings("QSyncthingTray", "qst")
  {}

  AppSettings(const AppSettings&) = delete;
  AppSettings& operator=(const AppSettings&) = delete;
  AppSettings(const AppSettings&&) = delete;
  AppSettings&& operator=(const AppSettings&&) = delete;

  template <typename T, typename U, typename ... TArgs>
  void setValues(std::pair<T,U> uiElement, TArgs...   Elements)
  {
    mSettings.setValue(uiElement.first, uiElement.second);
    setValues(std::forward<TArgs>(Elements)...);
  }

  
  template <typename T, typename U>
  void setValues(std::pair<T,U> uiElement)
  {
    emit(settingsUpdated());
    mSettings.setValue(uiElement.first, uiElement.second);
  }


  QVariant value(const QString& key)
  {
    return mSettings.value(key);
  }

signals:
  void settingsUpdated();

private:
  settings::SettingsMigrator mSettingsMigrator;
  QSettings mSettings;
};
} // namespace settings
} // namespace qst

#endif //appsettings_h
