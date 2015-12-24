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

namespace mfk
{
namespace settings
{

class StartupTab : public QWidget
{
  Q_OBJECT
  
public:
  StartupTab(std::shared_ptr<mfk::connector::SyncConnector> pSyncConnector);
  ~StartupTab();
  bool isPausingProcessRunning();
  void spawnSyncthingApp();
  void saveSettings();
  
private slots:
  void launchSyncthingBoxChanged(int state);
  void launchINotifyBoxChanged(int state);
  void pathEnterPressed();
  void showFileBrowser();
  void showINotifyFileBrowser();
  
private:
  void loadSettings();
  void initGUI();

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
  
  bool mShouldLaunchSyncthing;
  bool mShouldLaunchINotify;
  std::string mCurrentSyncthingPath;
  std::string mCurrentINotifyPath;

  std::shared_ptr<mfk::connector::SyncConnector> mpSyncConnector;
  mfk::sysutils::SystemUtility systemUtil;
  QSettings mSettings;
};

} // monitor
} // mfk

#endif
