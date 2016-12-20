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

#ifndef SYNCCONNECTIONSETTINGS_H
#define SYNCCONNECTIONSETTINGS_H

#include <QByteArray>
#include <QString>

namespace qst
{
namespace settings
{
struct SyncConnectionSettings
{
  QString url;
  QString username;
  QString password;
  QByteArray apiKey;
  int pollIntervalMs = 1000;
};

} // namespace settings
} // namespace qst
#endif
