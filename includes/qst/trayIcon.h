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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include <QIcon>
#include <QMovie>
#include <qst/appsettings.hpp>
#include <qst/trayMenu.h>

#include <memory>

namespace qst
{
namespace ui
{
class TrayIcon : public QSystemTrayIcon
{
  Q_OBJECT
public:
  TrayIcon(QObject *parent = nullptr);

public slots:
  void setTrayIcon(int index);
  void onStartAnimation(const bool animate);
  void iconActivated(const QSystemTrayIcon::ActivationReason reason);
  bool eventFilter(QObject *object, QEvent *event);

private:
  void onUpdateIcon();
  QMenu* mpMenu;
  settings::AppSettingsSPtr mpAppSettings;
  std::shared_ptr<TrayMenu> mpTrayMenu;
  std::unique_ptr<QMovie> mpAnimatedIconMovie;
  SyncViewWidgetAction mTrayMenuAction;
  bool mShouldStopAnimation = false;
};
}
}
#endif
