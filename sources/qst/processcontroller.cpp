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
//------------------------------------------------------------------------------------//

#include <qst/processcontroller.h>
#include <QDir>

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

namespace qst
{
namespace process
{


//------------------------------------------------------------------------------------//

ProcessController::ProcessController(std::shared_ptr<settings::AppSettings> appSettings) :
  mpAppSettings(appSettings)
{
  connect(mpAppSettings.get(), &settings::AppSettings::settingsUpdated,
    this, &ProcessController::onSettingsUpdated);
  spawnSyncthingProcess();
  spawnINotifyProcess();
}


//------------------------------------------------------------------------------------//

void ProcessController::syncThingProcessSpawned(QProcess::ProcessState newState)
{
  switch (newState)
  {
    case QProcess::Running:
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::SPAWNED}}));
      break;
    case QProcess::NotRunning:
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::NOT_RUNNING}}));
      break;
    default:
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::NOT_RUNNING}}));
  }
}


//------------------------------------------------------------------------------------//

void ProcessController::checkAndSpawnINotifyProcess()
{
  if (mpINotifyProcess)
  {
    mpINotifyProcess->terminate();
  }
  if (mpAppSettings->value(kLaunchInotifyStartupId).toBool())
  {
    const QString filepath = mpAppSettings->value(kInotifyPathId).toString();
    if (!mSystemUtil.isBinaryRunning(std::string("syncthing-inotify")))
    {
      mpINotifyProcess = std::unique_ptr<QProcess>(new QProcess(this));
      QString processPath = QDir::toNativeSeparators(filepath);
      connect(mpINotifyProcess.get(), SIGNAL(stateChanged(QProcess::ProcessState)),
              this, SLOT(notifyProcessSpawned(QProcess::ProcessState)));
      mpINotifyProcess->start(processPath, QStringList(), QIODevice::Unbuffered);
    }
    else
    {
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::ALREADY_RUNNING}}));
    }
  }
  else
  {
    shutdownINotifyProcess();
  }
}


//------------------------------------------------------------------------------------//

void ProcessController::notifyProcessSpawned(QProcess::ProcessState newState)
{
  switch (newState)
  {
    case QProcess::Running:
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::SPAWNED}}));
      break;
    case QProcess::NotRunning:
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::NOT_RUNNING}}));
      break;
    default:
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::NOT_RUNNING}}));
  }
}


//------------------------------------------------------------------------------------//

void ProcessController::setPaused(const bool paused)
{
  if (paused)
  {
    shutdownINotifyProcess();
    emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::PAUSED}}));
    if (mpAppSettings->value(kLaunchInotifyStartupId).toBool())
    {
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::PAUSED}}));
    }
  }
  else
  {
    spawnSyncthingProcess();
    spawnINotifyProcess();
  }
}


//------------------------------------------------------------------------------------//

void ProcessController::onSettingsUpdated()
{
  spawnSyncthingProcess();
  spawnINotifyProcess();
}


//------------------------------------------------------------------------------------//

void ProcessController::spawnSyncthingProcess()
{
  QString syncthingFilePath = mpAppSettings->value(kSyncthingPathId).toString();
  if (mpAppSettings->value(kLaunchSyncthingStartupId).toBool())
  {
    if (!mSystemUtil.isBinaryRunning(std::string("syncthing")))
    {
      mpSyncthingProcess = std::unique_ptr<QProcess>(new QProcess(this));
      connect(mpSyncthingProcess.get(), SIGNAL(stateChanged(QProcess::ProcessState)),
              this, SLOT(syncThingProcessSpawned(QProcess::ProcessState)));
      QString processPath = QDir::toNativeSeparators(syncthingFilePath);
      QStringList launchArgs;
      launchArgs << "-no-browser";
      mpSyncthingProcess->start(processPath, launchArgs);
    }
    else
    {
      emit(onProcessSpawned({{kSyncthingIdentifier, ProcessState::ALREADY_RUNNING}}));
    }
  }
}


//------------------------------------------------------------------------------------//

void ProcessController::spawnINotifyProcess()
{
  if (mpAppSettings->value(kLaunchInotifyStartupId).toBool())
  {
    if (!mSystemUtil.isBinaryRunning(std::string("syncthing-inotify")))
    {
      mpINotifyProcess = std::unique_ptr<QProcess>(new QProcess(this));
      QString processPath = QDir::toNativeSeparators(
         mpAppSettings->value(kInotifyPathId).toString());
      connect(mpINotifyProcess.get(), SIGNAL(stateChanged(QProcess::ProcessState)),
              this, SLOT(notifyProcessSpawned(QProcess::ProcessState)));
      mpINotifyProcess->start(processPath, QStringList(), QIODevice::Unbuffered);
    }
    else
    {
      emit(onProcessSpawned({{kNotifyIdentifier, ProcessState::ALREADY_RUNNING}}));
    }
  }
  else
  {
    shutdownINotifyProcess();
  }
}


//------------------------------------------------------------------------------------//

void ProcessController::shutdownINotifyProcess()
{
  if (mpINotifyProcess != nullptr &&
      mpINotifyProcess->state() == QProcess::Running)
  {
    mpINotifyProcess->kill();
  }
}


//------------------------------------------------------------------------------------//

ProcessController::~ProcessController()
{
  killProcesses();
}


//------------------------------------------------------------------------------------//

void ProcessController::killProcesses()
{
  if (mpSyncthingProcess != nullptr
      && mpAppSettings->value("ShutdownOnExit").toBool())
  {
    mpSyncthingProcess->waitForFinished();
  }
  if (mpINotifyProcess != nullptr
      && mpINotifyProcess->state() == QProcess::Running)
  {
    mpINotifyProcess->terminate();
    mpINotifyProcess->waitForFinished();
  }
}

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

} // process namespace
} // qst namespace
