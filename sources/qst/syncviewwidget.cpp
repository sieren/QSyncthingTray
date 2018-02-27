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


#include <qst/syncviewwidget.h>
#include "ui_syncviewwidget.h"

namespace qst
{
namespace ui
{
SyncViewWidget::SyncViewWidget(settings::AppSettingsSPtr pAppSettings,
 TrayMenu* pTrayMenu, QWidget* parent) :
    QWidget(parent)
  , mpAppSettings(pAppSettings)
  , mpTrayMenu(pTrayMenu)
  , mpUi(new Ui::SyncViewWidget)
  , mConnections(10)
{
  mpUi->setupUi(this);
  initializeConnections();
//  mpUi->dirsView->set
}

void SyncViewWidget::setupConnections()
{
  connect(mConnections.back().get(), &connector::SyncConnection::trafficChanged,
    this, &SyncViewWidget::onTrafficChanged);
}
void SyncViewWidget::initializeConnections()
{
  // TODO: Support multiple connections
  // This requires changing the Settings Layout
  // to store multiple connections and needs migration
  // from single-connection to multi.
  using namespace settings;
  SyncConnectionSettings connSettings;
  connSettings.url = mpAppSettings->value(kUrlId).toString();
  connSettings.username = mpAppSettings->value(kUserNameId).toString();
  connSettings.password = mpAppSettings->value(kPasswordId).toString();
  connSettings.apiKey = mpAppSettings->value(kApiKeyId).toByteArray();

  using namespace connector;
  mConnections.emplace_back(
    std::unique_ptr<SyncConnection>(new SyncConnection(std::move(connSettings))));
  setupConnections();
}

void SyncViewWidget::onTrafficChanged()
{
  // TODO
  // show total traffic in UI
  auto connection = mConnections.back().get();
  if(mpUi->trafficFormWidget->isHidden()) {
      return;
  }
  static const QString unknownStr(tr("unknown"));
  if(connection->isConnected())
  {
    auto inTraff = QString("%1 ").arg(connection->totalInTraff());
    auto inRate = utilities::trafficToString(connection->totalInRate());
    mpUi->inTrafficLabel->setText(inRate);

    auto outRate = utilities::trafficToString(connection->totalOutRate());
    mpUi->outTrafficLabel->setText(outRate);
  }
  else
  {
      mpUi->inTrafficLabel->setText("Unknown");
      mpUi->outTrafficLabel->setText("Unknown");
  }
}
SyncViewWidget::~SyncViewWidget()
{

}
}
}
