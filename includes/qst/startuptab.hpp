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

#ifndef __startuptab__
#define __startuptab__


#include <stdio.h>

#include <QtGui>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QTableWidget>
#include <QSharedPointer>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <iostream>
#include <map>
#include <qst/appsettings.hpp>
#include <qst/processcontroller.h>
#include <qst/processmonitor.hpp>
#include "syncconnector.h"

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
QT_END_NAMESPACE

namespace qst
{
namespace settings
{

class StartupTab : public QWidget
{
  Q_OBJECT
  
public:
  StartupTab(std::shared_ptr<process::ProcessController> pProcController,
    std::shared_ptr<settings::AppSettings> appSettings);
  ~StartupTab();

public slots:
  void saveSettings();

private slots:
  void launchSyncthingBoxChanged(int state);
  void launchINotifyBoxChanged(int state);
  void shutdownOnExitBoxChanged(int state);
  void showFileBrowser();
  void showINotifyFileBrowser();
  void processSpawnedChanged(const ProcessStateInfo& info);
  
private:
  void updateLabelWithState(QLabel* label, const ProcessState &state);
  void startProcesses();
  void loadSettings();
  void initGUI();
  void displayPathNotFound(const QString& processName);
  template <typename T, typename ... TArgs>
  void hideShowElements(bool show, T uiElement, TArgs... Elements);

  template <typename T>
  void hideShowElements(bool show, T uiElement);
  
  QGroupBox *mpFilePathGroupBox;
  QLineEdit *mpFilePathLine;
  QPushButton *mpFilePathBrowse;
  QLabel *mpAppSpawnedLabel;
  QCheckBox *mpShouldLaunchSyncthingBox;
  
  QGroupBox *mpiNotifyGroupBox;
  QLineEdit *mpINotifyFilePath;
  QPushButton *mpINotifyBrowse;
  QLabel *mpINotifySpawnedLabel;
  QCheckBox *mpShouldLaunchINotify;
  QCheckBox *mpShutdownOnExitBox;
  
  bool mShouldLaunchSyncthing;
  bool mShouldLaunchINotify;
  bool mShouldShutdownOnExit;

  std::shared_ptr<process::ProcessController> mpProcController;
  qst::sysutils::SystemUtility systemUtil;
  std::shared_ptr<settings::AppSettings> mpAppSettings;
};

} // monitor
} // qst

#endif
