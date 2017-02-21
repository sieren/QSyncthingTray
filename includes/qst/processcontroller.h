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

#ifndef processcontroller_h
#define processcontroller_h
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <map>
#include <memory>
#include <qst/appsettings.hpp>

enum class ProcessState
{
  SPAWNED,
  NOT_RUNNING,
  ALREADY_RUNNING,
  PAUSED
};

using ProcessStateInfo = std::map<std::string, ProcessState>;

namespace
{
  static const std::string kSyncthingIdentifier{"syncthing"};
  static const std::string kNotifyIdentifier{"inotify"};
} // anon namespace

namespace qst
{
namespace process
{

class ProcessController : public QObject
{
  Q_OBJECT
public:
  ProcessController(std::shared_ptr<settings::AppSettings> appSettings);
  ProcessController(ProcessController&&) = delete;
  ProcessController(const ProcessController&) = delete;
  ProcessController& operator=(const ProcessController&) = delete;
  ~ProcessController();
  void setPaused(const bool paused);
  void checkAndSpawnINotifyProcess();
  ProcessState getSyncthingState() const;
  ProcessState getINotifyState() const;

signals:
  void onNothing(int bla);
  void onProcessSpawned(ProcessStateInfo procState);

private slots:
  void syncThingProcessSpawned(QProcess::ProcessState newState);
  void notifyProcessSpawned(QProcess::ProcessState newState);
  void onSettingsUpdated();

private:
  void spawnSyncthingProcess();
  void spawnINotifyProcess();
  void killProcesses();
  void shutdownINotifyProcess();
  qst::sysutils::SystemUtility mSystemUtil;
  std::shared_ptr<settings::AppSettings> mpAppSettings;
  std::unique_ptr<QProcess> mpSyncthingProcess;
  std::unique_ptr<QProcess> mpINotifyProcess;
};

} // process namespace
} // qst namespace

#endif /* processcontroller_h */
