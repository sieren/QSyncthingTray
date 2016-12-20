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

#ifndef SYNCDEVICE_H
#define SYNCDEVICE_H

#include <chrono>

namespace qst
{
namespace model
{
enum class SyncDeviceStatus
{
  Unknown,
  Disconnected,
  Self,
  Idle,
  Syncing,
  OutOfSync,
  Rejected
};

struct SyncDevice
{
  explicit SyncDevice(const QString& id, const QString& name) :
    id(id)
  , name(name)
  {};

  QString id;
  QString name;
  QString compression;
  SyncDeviceStatus status;
  std::time_t lastSeen;
};
}
}

#endif
