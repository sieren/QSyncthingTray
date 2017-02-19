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


#include <qst/startuptab.hpp>
#include <qst/utilities.hpp>
#include <QFileDialog>

namespace qst
{
namespace settings
{

StartupTab::StartupTab(std::shared_ptr<process::ProcessController> pProcController,
  std::shared_ptr<settings::AppSettings> appSettings) :
    mpProcController(pProcController)
  , mpAppSettings(appSettings)
{
  
  loadSettings();
  connect(mpProcController.get(), &process::ProcessController::onProcessSpawned,
    this, &StartupTab::processSpawnedChanged);
  initGUI();
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
  
  mpFilePathLine = new QLineEdit(mpAppSettings->value(kSyncthingPathId).toString());
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
  connect(mpFilePathLine, SIGNAL(returnPressed()), this, SLOT(saveSettings()));
  connect(mpShouldLaunchSyncthingBox, SIGNAL(stateChanged(int)), this,
          SLOT(launchSyncthingBoxChanged(int)));
  
  //
  // INOTIFY BINARY PATH
  //
  
  mpiNotifyGroupBox = new QGroupBox(tr("iNotify Application"));

  mpINotifySpawnedLabel = new QLabel(tr("Not started"));
  mpShouldLaunchINotify = new QCheckBox(tr("Launch iNotify"));
  Qt::CheckState iNotifylaunchState = mShouldLaunchINotify ? Qt::Checked : Qt::Unchecked;
  mpShouldLaunchINotify->setCheckState(iNotifylaunchState);
  QGridLayout *iNotifyLayout = new QGridLayout;

  mpINotifyFilePath = new QLineEdit(mpAppSettings->value(kInotifyPathId).toString());
  mpINotifyBrowse = new QPushButton(tr("Browse"));

  iNotifyLayout->addWidget(mpINotifyFilePath,2, 0, 1, 4);
  iNotifyLayout->addWidget(mpINotifyBrowse,3, 0, 1, 1);
  iNotifyLayout->addWidget(mpINotifySpawnedLabel, 3, 1, 1, 1);
  iNotifyLayout->addWidget(mpShouldLaunchINotify, 0, 0);

  mpiNotifyGroupBox->setLayout(iNotifyLayout);
  mpiNotifyGroupBox->setMinimumWidth(400);
  mpiNotifyGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  
  connect(mpINotifyBrowse, SIGNAL(clicked()), this, SLOT(showINotifyFileBrowser()));
  connect(mpINotifyFilePath, SIGNAL(returnPressed()), this, SLOT(saveSettings()));
  connect(mpShouldLaunchINotify, SIGNAL(stateChanged(int)), this,
    SLOT(launchINotifyBoxChanged(int)));
  connect(mpShutdownOnExitBox, SIGNAL(stateChanged(int)), this,
    SLOT(shutdownOnExitBoxChanged(int)));
  

  launcherLayout->addWidget(mpFilePathGroupBox);
  launcherLayout->addWidget(mpiNotifyGroupBox);
  setLayout(launcherLayout);
}

//------------------------------------------------------------------------------------//

void StartupTab::processSpawnedChanged(const ProcessStateInfo& info)
{
  auto syncProcess = info.find(kSyncthingIdentifier);
  if (syncProcess != info.end())
  {
    updateLabelWithState(mpAppSpawnedLabel, syncProcess->second);
  }
  auto notifyprocess = info.find(kNotifyIdentifier);
  if (notifyprocess != info.end())
  {
    updateLabelWithState(mpINotifySpawnedLabel, notifyprocess->second);
  }
}


//------------------------------------------------------------------------------------//

void StartupTab::updateLabelWithState(QLabel* label, const ProcessState &state)
{
  switch (state) {
    case ProcessState::SPAWNED:
      label->setText(tr("Status: Launched"));
      break;
    case ProcessState::NOT_RUNNING:
      label->setText(tr("Status: Not started"));
      break;
    case ProcessState::ALREADY_RUNNING:
      label->setText(tr("Already Running"));
      break;
    case ProcessState::PAUSED:
      label->setText(tr("Paused"));
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
  if (!filename.isEmpty())
  {
    mpFilePathLine->setText(filename);
  }
  saveSettings();
}


//------------------------------------------------------------------------------------//

void StartupTab::showINotifyFileBrowser()
{
  QString filename = QFileDialog::getOpenFileName(this,
    tr("Open iNotify"), "", tr(""));
  if (filename.toStdString() != "")
  {
    mpINotifyFilePath->setText(filename);
  }
  saveSettings();
}
  
//------------------------------------------------------------------------------------//

void StartupTab::launchSyncthingBoxChanged(int state)
{
  hideShowElements(state == Qt::Checked, mpFilePathLine, mpFilePathBrowse,
    mpAppSpawnedLabel);
  mShouldLaunchSyncthing = state == Qt::Checked ? true : false;
  saveSettings();
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
}


//------------------------------------------------------------------------------------//

void StartupTab::saveSettings()
{
  using namespace std;
  // Check Filepath Validity
  if (!mpFilePathLine->text().isEmpty() &&
      !utilities::checkIfFileExists(mpFilePathLine->text()))
  {
    displayPathNotFound("Syncthing");
  }

  // Check Filepath Validity
  if (!mpINotifyFilePath->text().isEmpty() &&
      !utilities::checkIfFileExists(mpINotifyFilePath->text()))
  {
    displayPathNotFound("syncthing-inotify");
  }
  mpAppSettings->setValues(
    make_pair(kSyncthingPathId, mpFilePathLine->text()),
    make_pair(kLaunchSyncthingStartupId, mShouldLaunchSyncthing),
    make_pair(kInotifyPathId, mpINotifyFilePath->text()),
    make_pair(kLaunchInotifyStartupId, mShouldLaunchINotify),
    make_pair(kShutDownExitId, mShouldShutdownOnExit));
}


//------------------------------------------------------------------------------------//

void StartupTab::loadSettings()
{
  mShouldLaunchSyncthing = mpAppSettings->value(kLaunchSyncthingStartupId).toBool();
  mShouldLaunchINotify = mpAppSettings->value(kLaunchInotifyStartupId).toBool();
  mShouldShutdownOnExit = mpAppSettings->value(kShutDownExitId).toBool();
}


//------------------------------------------------------------------------------------//

void StartupTab::displayPathNotFound(const QString &processName)
{
  QMessageBox msgBox;
  msgBox.setText("Could not find " + processName + ".");
  msgBox.setInformativeText("Are you sure the path is correct?");
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.exec();
}


//------------------------------------------------------------------------------------//

StartupTab::~StartupTab()
{
  disconnect(mpProcController.get(),
          &process::ProcessController::onProcessSpawned, this,
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
