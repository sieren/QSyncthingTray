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

#ifndef TRAYMENU_H
#define TRAYMENU_H

#include <qst/appsettings.hpp>
#include <qst/syncviewwidget.h>
#include <QMenu>
#include <QObject>
#include <memory>

namespace qst
{
namespace ui
{
class TrayIcon;
class TrayMenu : public QMenu
{
public:
  TrayMenu(QWidget *parent = nullptr);
  TrayMenu(TrayIcon* pIcon, settings::AppSettingsSPtr pSettings, QWidget *parent = nullptr);
  
  QSize sizeHint() const;
  inline SyncViewWidget* widget()
  {
    return mpSyncViewWidget.get();
  }

  inline TrayIcon* trayIcon()
  {
    return mpTrayIcon;
  }
private:
  TrayIcon* mpTrayIcon;
  std::unique_ptr<SyncViewWidget> mpSyncViewWidget;
};
}
}
#endif
