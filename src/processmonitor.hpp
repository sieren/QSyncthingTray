//
//  processmonitor.hpp
//  QSyncthingTray
//
//  Created by Matthias Frick on 11.10.2015.
//
//

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

namespace mfk
{
namespace monitor
{

class ProcessMonitor : public QWidget
{
  Q_OBJECT
  
public:
  ProcessMonitor(std::shared_ptr<mfk::connector::SyncConnector> pSyncConnector);
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
  std::shared_ptr<mfk::connector::SyncConnector> mpSyncConnector;
  QSettings mSettings;
  QStringList mProcessList;
  
  std::unique_ptr<QTimer> mpProcessCheckTimer;
  
  mfk::sysutils::SystemUtility systemUtil;
};

} // monitor
} // mfk

#endif