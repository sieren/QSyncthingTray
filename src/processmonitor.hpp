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

#ifndef __systray__processmonitor__
#define __systray__processmonitor__


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

namespace qst
{
namespace monitor
{

class ProcessMonitor : public QWidget
{
  Q_OBJECT

public:
  ProcessMonitor(std::shared_ptr<qst::connector::SyncConnector> pSyncConnector);
  virtual ~ProcessMonitor() = default;
  bool isPausingProcessRunning();

private slots:
  void addButtonClicked();
  void deleteButtonClicked();
  void cellSelected(int nRow, int nColumn);
  void checkProcessList();

private:
  void loadSettings();
  void saveSettings();
  void refreshTable();
  QTableWidget *mpProcessTable;
  QLineEdit *mpProcessLineEdit;
  QPushButton *mpAddToListButton;
  QPushButton *mpDeleteFromListButton;
  std::shared_ptr<qst::connector::SyncConnector> mpSyncConnector;
  QSettings mSettings;
  QStringList mProcessList;

  std::unique_ptr<QTimer> mpProcessCheckTimer;

  qst::sysutils::SystemUtility systemUtil;
};

} // monitor
} // qst

#endif
