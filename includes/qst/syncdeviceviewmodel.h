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

#ifndef SYNCDEVICEMODEL_H
#define SYNCDEVICEMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <QString>
#include <vector>

namespace qst
{
namespace connector
{
  class SyncConnection;
}

namespace model
{
struct SyncDevice;
class SyncDeviceModel : public QObject
{
public:
  SyncDeviceModel(connector::SyncConnection &connection, QObject *parent = nullptr);
  const SyncDevice *deviceInfo(const QModelIndex &index) const;

public Q_SLOTS:
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;

private Q_SLOTS:
  void onConfigChanged();
  void onDevicesChanged();
  void onDeviceStatusChanged(const SyncDevice&, int index);

private:
  connector::SyncConnection &mConnection;
  const std::vector<SyncDevice> &mDevices;
};
}
}

#endif

