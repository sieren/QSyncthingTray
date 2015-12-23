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

namespace mfk
{
namespace settings
{

StartupTab::StartupTab(std::shared_ptr<mfk::connector::SyncConnector> pSyncConnector) :
    mpSyncConnector(pSyncConnector)
  , mSettings("sieren", "QSyncthingTray")
{
  
  loadSettings();

  initGUI();
  
  mpSyncConnector->setProcessSpawnedCallback([&](kSyncthingProcessState state)
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
   });
  
  mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath, mShouldLaunchSyncthing);
  mpSyncConnector->spawnINotifyProcess(mCurrentINotifyPath, mShouldLaunchINotify);
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
  
  filePathLayout->addWidget(mpFilePathLine,2, 0, 1, 4);
  filePathLayout->addWidget(mpFilePathBrowse,3, 0, 1, 1);
  filePathLayout->addWidget(mpAppSpawnedLabel, 3, 1, 1, 1);
  
  filePathLayout->addWidget(mpShouldLaunchSyncthingBox, 0, 0);
  mpFilePathGroupBox->setLayout(filePathLayout);
  mpFilePathGroupBox->setMinimumWidth(400);
  mpFilePathGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  
  launchSyncthingBoxChanged(launchState);
  
  
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
  
  launchINotifyBoxChanged(iNotifylaunchState);
  
  
  connect(mpINotifyBrowse, SIGNAL(clicked()), this, SLOT(showINotifyFileBrowser()));
  connect(mpINotifyFilePath, SIGNAL(returnPressed()), this, SLOT(pathEnterPressed()));
  connect(mpShouldLaunchINotify, SIGNAL(stateChanged(int)), this,
          SLOT(launchINotifyBoxChanged(int)));
  

  launcherLayout->addWidget(mpFilePathGroupBox);
  launcherLayout->addWidget(mpiNotifyGroupBox);
  setLayout(launcherLayout);

}


//------------------------------------------------------------------------------------//

void StartupTab::showFileBrowser()
{
  QString filename = QFileDialog::getOpenFileName(this,
    tr("Open Syncthing"), "", tr(""));
  mCurrentSyncthingPath = filename.toStdString();
  mpFilePathLine->setText(filename);
  saveSettings();
  spawnSyncthingApp();
}


//------------------------------------------------------------------------------------//

void StartupTab::showINotifyFileBrowser()
{
  QString filename = QFileDialog::getOpenFileName(this,
    tr("Open iNotify"), "", tr(""));
  mCurrentINotifyPath = filename.toStdString();
  mpINotifyFilePath->setText(filename);
  saveSettings();
  spawnSyncthingApp();
}
  
//------------------------------------------------------------------------------------//

void StartupTab::launchSyncthingBoxChanged(int state)
{
  if (state)
  {
    mpFilePathLine->show();
    mpFilePathBrowse->show();
    mpAppSpawnedLabel->show();
    mShouldLaunchSyncthing = true;
  }
  else
  {
    mpFilePathLine->hide();
    mpFilePathBrowse->hide();
    mpAppSpawnedLabel->hide();
    mShouldLaunchSyncthing = false;
  }
}


//------------------------------------------------------------------------------------//


void StartupTab::launchINotifyBoxChanged(int state)
{
  if (state)
  {
    mpINotifyFilePath->show();
    mpINotifyBrowse->show();
    mShouldLaunchINotify = true;
  }
  else
  {
    mpINotifyFilePath->hide();
    mpINotifyBrowse->hide();
    mShouldLaunchSyncthing = false;
  }
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
  if (mSettings.value("launchINotifyAtStartup").toBool() != mShouldLaunchINotify)
  {
    mpSyncConnector->spawnINotifyProcess(mCurrentINotifyPath, mShouldLaunchINotify);
  }
  mSettings.setValue("launchINotifyAtStartup", mShouldLaunchINotify);
}


//------------------------------------------------------------------------------------//

void StartupTab::loadSettings()
{
  mCurrentSyncthingPath = mSettings.value("syncthingpath").toString().toStdString();
  mShouldLaunchSyncthing = mSettings.value("launchSyncthingAtStartup").toBool();
  mCurrentINotifyPath = mSettings.value("inotifypath").toString().toStdString();
  mShouldLaunchINotify = mSettings.value("launchINotifyAtStartup").toBool();
}


//------------------------------------------------------------------------------------//

void StartupTab::pathEnterPressed()
{
  mCurrentSyncthingPath = mpFilePathLine->text().toStdString();
  mCurrentINotifyPath = mpINotifyFilePath->text().toStdString();
  if (mSettings.value("syncthingpath").toString().toStdString() != mCurrentSyncthingPath)
  {
    mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath, true, true);
  }
  if (mSettings.value("inotifypath").toString().toStdString() != mCurrentINotifyPath)
  {
    mpSyncConnector->spawnINotifyProcess(mCurrentINotifyPath, true, true);
  }
}


//------------------------------------------------------------------------------------//

void StartupTab::spawnSyncthingApp()
{
  mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath, mShouldLaunchSyncthing);
}

} // settings
} // mfk