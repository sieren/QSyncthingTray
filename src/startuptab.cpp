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

#include "startuptab.hpp"
#include <QFileDialog>

namespace qst
{
namespace settings
{

StartupTab::StartupTab(std::shared_ptr<qst::connector::SyncConnector> pSyncConnector) :
    mpSyncConnector(pSyncConnector)
  , mSettings("sieren", "QSyncthingTray")
{
  
  loadSettings();

  initGUI();
  
  connect(pSyncConnector.get(), &connector::SyncConnector::onProcessSpawned, this,
    &StartupTab::processSpawnedChanged);
  
  mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath, mShouldLaunchSyncthing);
}


//------------------------------------------------------------------------------------//

void StartupTab::initGUI()
{
  QVBoxLayout *launcherLayout = new QVBoxLayout;
  launcherLayout->setAlignment(Qt::AlignTop);
  
  //
  // SYNCTHING BINARY PATH
  //
  
  mpFilePathGroupBox = new QGroupBox(tr("Syncthing Application"));
  
  mpShouldLaunchSyncthingBox = new QCheckBox(tr("Launch Syncthing"));
  Qt::CheckState launchState = mShouldLaunchSyncthing ? Qt::Checked : Qt::Unchecked;
  mpShouldLaunchSyncthingBox->setCheckState(launchState);
  QGridLayout *filePathLayout = new QGridLayout;
  
  mpFilePathLine = new QLineEdit(mCurrentSyncthingPath.c_str());
  //  mpFilePathLine->setFixedWidth(maximumWidth / devicePixelRatio());
  mpFilePathBrowse = new QPushButton(tr("Browse"));
  
  mpAppSpawnedLabel = new QLabel(tr("Not started"));
  
  mpShutdownOnExitBox = new QCheckBox(tr("Shutdown on Exit"));
  Qt::CheckState shutdownState = mShouldShutdownOnExit ? Qt::Checked : Qt::Unchecked;
  mpShutdownOnExitBox->setCheckState(shutdownState);
  
  filePathLayout->addWidget(mpFilePathLine,2, 0, 1, 4);
  filePathLayout->addWidget(mpFilePathBrowse,3, 0, 1, 1);
  filePathLayout->addWidget(mpAppSpawnedLabel, 3, 1, 1, 1);
  
  filePathLayout->addWidget(mpShouldLaunchSyncthingBox, 0, 0);
  filePathLayout->addWidget(mpShutdownOnExitBox, 4, 0, 1, 2);
  mpFilePathGroupBox->setLayout(filePathLayout);
  mpFilePathGroupBox->setMinimumWidth(400);
  mpFilePathGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  
  connect(mpFilePathBrowse, SIGNAL(clicked()), this, SLOT(showFileBrowser()));
  connect(mpFilePathLine, SIGNAL(returnPressed()), this, SLOT(pathEnterPressed()));
  connect(mpShouldLaunchSyncthingBox, SIGNAL(stateChanged(int)), this,
          SLOT(launchSyncthingBoxChanged(int)));
  
  //
  // INOTIFY BINARY PATH
  //
  
  mpiNotifyGroupBox = new QGroupBox(tr("iNotify Application"));
  
  mpShouldLaunchINotify = new QCheckBox(tr("Launch iNotify"));
  Qt::CheckState iNotifylaunchState = mShouldLaunchINotify ? Qt::Checked : Qt::Unchecked;
  mpShouldLaunchINotify->setCheckState(iNotifylaunchState);
  QGridLayout *iNotifyLayout = new QGridLayout;

  mpINotifyFilePath = new QLineEdit(mCurrentINotifyPath.c_str());
  //  mpFilePathLine->setFixedWidth(maximumWidth / devicePixelRatio());
  mpINotifyBrowse = new QPushButton(tr("Browse"));

  iNotifyLayout->addWidget(mpINotifyFilePath,2, 0, 1, 4);
  iNotifyLayout->addWidget(mpINotifyBrowse,3, 0, 1, 1);
  iNotifyLayout->addWidget(mpShouldLaunchINotify, 0, 0);

  mpiNotifyGroupBox->setLayout(iNotifyLayout);
  mpiNotifyGroupBox->setMinimumWidth(400);
  mpiNotifyGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  
  connect(mpINotifyBrowse, SIGNAL(clicked()), this, SLOT(showINotifyFileBrowser()));
  connect(mpINotifyFilePath, SIGNAL(returnPressed()), this, SLOT(pathEnterPressed()));
  connect(mpShouldLaunchINotify, SIGNAL(stateChanged(int)), this,
    SLOT(launchINotifyBoxChanged(int)));
  connect(mpShutdownOnExitBox, SIGNAL(stateChanged(int)), this,
    SLOT(shutdownOnExitBoxChanged(int)));
  

  launcherLayout->addWidget(mpFilePathGroupBox);
  launcherLayout->addWidget(mpiNotifyGroupBox);
  setLayout(launcherLayout);

  launchSyncthingBoxChanged(launchState);
  launchINotifyBoxChanged(iNotifylaunchState);
  
}

//------------------------------------------------------------------------------------//

void StartupTab::processSpawnedChanged(kSyncthingProcessState state)
{
  switch (state) {
    case kSyncthingProcessState::SPAWNED:
      mpAppSpawnedLabel->setText(tr("Status: Launched"));
      break;
    case kSyncthingProcessState::NOT_RUNNING:
      mpAppSpawnedLabel->setText(tr("Status: Not started"));
      break;
    case kSyncthingProcessState::ALREADY_RUNNING:
      mpAppSpawnedLabel->setText(tr("Already Running"));
      break;
    case kSyncthingProcessState::PAUSED:
      mpAppSpawnedLabel->setText(tr("Paused"));
      break;
    default:
      break;
  }
}


//------------------------------------------------------------------------------------//

void StartupTab::showFileBrowser()
{
  QString filename = QFileDialog::getOpenFileName(this,
    tr("Open Syncthing"), "", tr(""));
  if (filename.toStdString() != "")
  {
    mCurrentSyncthingPath = filename.toStdString();
    mpFilePathLine->setText(filename);
  }
  saveSettings();
  spawnSyncthingApp();
}


//------------------------------------------------------------------------------------//

void StartupTab::showINotifyFileBrowser()
{
  QString filename = QFileDialog::getOpenFileName(this,
    tr("Open iNotify"), "", tr(""));
  if (filename.toStdString() != "")
  {
    mCurrentINotifyPath = filename.toStdString();
    mpINotifyFilePath->setText(filename);
  }
  saveSettings();
  spawnSyncthingApp();
}
  
//------------------------------------------------------------------------------------//

void StartupTab::launchSyncthingBoxChanged(int state)
{
  hideShowElements(state == Qt::Checked, mpFilePathLine, mpFilePathBrowse,
    mpAppSpawnedLabel);
  mShouldLaunchSyncthing = state == Qt::Checked ? true : false;
}


//------------------------------------------------------------------------------------//

void StartupTab::shutdownOnExitBoxChanged(int state)
{
  mShouldShutdownOnExit = state == Qt::Checked ? true : false;
  saveSettings();
}

//------------------------------------------------------------------------------------//


void StartupTab::launchINotifyBoxChanged(int state)
{
  hideShowElements(state == Qt::Checked, mpINotifyFilePath, mpINotifyBrowse);
  mShouldLaunchINotify = state == Qt::Checked ? true : false;
  saveSettings();
  pathEnterPressed();
  mpSyncConnector->checkAndSpawnINotifyProcess(false);
}


//------------------------------------------------------------------------------------//

void StartupTab::saveSettings()
{
  mCurrentSyncthingPath = mpFilePathLine->text().toStdString();
  if (mSettings.value("syncthingpath").toString().toStdString() != mCurrentSyncthingPath)
  {
    pathEnterPressed();
  }
  mSettings.setValue("syncthingpath", tr(mCurrentSyncthingPath.c_str()));
  if (mSettings.value("launchSyncthingAtStartup").toBool() != mShouldLaunchSyncthing)
  {
    mpSyncConnector->spawnSyncthingProcess(
      mCurrentSyncthingPath, mShouldLaunchSyncthing);
  }
  mSettings.setValue("launchSyncthingAtStartup", mShouldLaunchSyncthing);
  
  mCurrentINotifyPath = mpINotifyFilePath->text().toStdString();
  if (mSettings.value("inotifypath").toString().toStdString() != mCurrentINotifyPath)
  {
    pathEnterPressed();
  }
  mSettings.setValue("inotifypath", tr(mCurrentINotifyPath.c_str()));
  mSettings.setValue("launchINotifyAtStartup", mShouldLaunchINotify);
  mSettings.setValue("ShutdownOnExit", mShouldShutdownOnExit);
  mpSyncConnector->onSettingsChanged();
}


//------------------------------------------------------------------------------------//

void StartupTab::loadSettings()
{
  mCurrentSyncthingPath = mSettings.value("syncthingpath").toString().toStdString();
  mShouldLaunchSyncthing = mSettings.value("launchSyncthingAtStartup").toBool();
  mCurrentINotifyPath = mSettings.value("inotifypath").toString().toStdString();
  mShouldLaunchINotify = mSettings.value("launchINotifyAtStartup").toBool();
  mShouldShutdownOnExit = mSettings.value("ShutdownOnExit").toBool();
}


//------------------------------------------------------------------------------------//

void StartupTab::pathEnterPressed()
{
  saveSettings();
  mCurrentSyncthingPath = mpFilePathLine->text().toStdString();
  mCurrentINotifyPath = mpINotifyFilePath->text().toStdString();
  if (mSettings.value("syncthingpath").toString().toStdString() != mCurrentSyncthingPath)
  {
    mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath, true, true);
  }
  if (mSettings.value("inotifypath").toString().toStdString() != mCurrentINotifyPath)
  {
    mpSyncConnector->checkAndSpawnINotifyProcess(true);
  }
}


//------------------------------------------------------------------------------------//

void StartupTab::spawnSyncthingApp()
{
  mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath, mShouldLaunchSyncthing);
}


//------------------------------------------------------------------------------------//

StartupTab::~StartupTab()
{
  disconnect(mpSyncConnector.get(), &connector::SyncConnector::onProcessSpawned, this,
          &StartupTab::processSpawnedChanged);
}


//------------------------------------------------------------------------------------//

template <typename T, typename ... TArgs>
void StartupTab::hideShowElements(bool show, T uiElement, TArgs...   Elements)
{
  if (show)
  {
    uiElement->show();
  }
  else
  {
    uiElement->hide();
  }
  hideShowElements(show, std::forward<TArgs>(Elements)...);
}


//------------------------------------------------------------------------------------//

template <typename T>
void StartupTab::hideShowElements(bool show, T uiElement)
{
  if (show)
  {
    uiElement->show();
  }
  else
  {
    uiElement->hide();
  }
}

//------------------------------------------------------------------------------------//

} // settings
} // qst
