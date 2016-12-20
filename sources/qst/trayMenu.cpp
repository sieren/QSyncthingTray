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


#include <qst/trayMenu.h>
#include <qst/syncviewwidget.h>

#include <QHBoxLayout>

namespace qst
{
namespace ui
{
  TrayMenu::TrayMenu(TrayIcon* pIcon, settings::AppSettingsSPtr pSettings, QWidget *parent) :
    QMenu(parent)
  , mpTrayIcon(pIcon)
  , mpSyncViewWidget(new SyncViewWidget(pSettings, this))
  {
    auto *menuLayout = new QHBoxLayout;
    menuLayout->setMargin(0), menuLayout->setSpacing(0);
  //  menuLayout->addWidget(mpSyncViewWidget.get());
    setLayout(menuLayout);
    setPlatformMenu(nullptr);
  }
QSize TrayMenu::sizeHint() const
{
  return QSize(120,120);
}

}
}
