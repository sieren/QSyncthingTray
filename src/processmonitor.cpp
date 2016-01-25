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


#include "processmonitor.hpp"
#include <QApplication>
#include <QHeaderView>

using namespace qst::monitor;

ProcessMonitor::ProcessMonitor(std::shared_ptr<qst::connector::SyncConnector> pSyncConnector)
  : mpSyncConnector(pSyncConnector)
  , mSettings("sieren", "QSyncthingTray")
{
  loadSettings();
  QLabel *descriptionLabel = new QLabel;
  descriptionLabel->setText("A list of processes which, when run, will pause Syncthing.");
  QFont labelFont = descriptionLabel->font();
  labelFont.setPointSize(10);
  descriptionLabel->setFont(labelFont);
  QStringList tableHeaders;
  tableHeaders << "Process";

  mpProcessTable = new QTableWidget();
  mpProcessTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  refreshTable();
  mpProcessTable->setHorizontalHeaderLabels(tableHeaders);
  connect(mpProcessTable, SIGNAL(cellDoubleClicked(int, int)),
    this, SLOT( cellSelected(int, int)));

  mpProcessLineEdit = new QLineEdit;
  mpAddToListButton = new QPushButton(tr("Add"));
  mpDeleteFromListButton = new QPushButton(tr("Delete"));
  connect(mpAddToListButton, SIGNAL(clicked()), this, SLOT(addButtonClicked()));
  connect(mpDeleteFromListButton, SIGNAL(clicked()), this, SLOT(deleteButtonClicked()));
  
  QLabel *addLabel = new QLabel;
  addLabel->setText("Add a process to the list by entering the name of the executable.");
  addLabel->setFont(labelFont);
  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(descriptionLabel, 0, 0, 1, 0);
  mainLayout->addWidget(mpProcessTable, 1, 0, 4 , 0);
  mainLayout->addWidget(addLabel, 5, 0, 1, 0);
  mainLayout->addWidget(mpProcessLineEdit, 6, 0, 1, 0);
  mainLayout->addWidget(mpAddToListButton, 7, 0, 1, 1);
  mainLayout->addWidget(mpDeleteFromListButton, 7, 1, 1, 1);
  setLayout(mainLayout);
  
  mpProcessCheckTimer = std::unique_ptr<QTimer>(new QTimer(this));
  connect(mpProcessCheckTimer.get(), SIGNAL(timeout()), this,
          SLOT(checkProcessList()));
  mpProcessCheckTimer->start(3000);
}


//------------------------------------------------------------------------------------//

bool ProcessMonitor::isPausingProcessRunning()
{
  for (auto process : mProcessList)
  {
    if (systemUtil.isBinaryRunning(process.toStdString()))
    {
      return true;
    }
  }
  return false;
}


//------------------------------------------------------------------------------------//

void ProcessMonitor::refreshTable()
{
  mpProcessTable->setRowCount(mProcessList.count());
  mpProcessTable->setColumnCount(1);
  mpProcessTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  mpProcessTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  mpProcessTable->setSelectionMode(QAbstractItemView::SingleSelection);
  mpProcessTable->setShowGrid(false);
  mpProcessTable->setStyleSheet("QTableView {selection-background-color: blue;}");
  
  for (int i = 0; i < mProcessList.count(); i++)
  {
    mpProcessTable->setItem(i, 0, new QTableWidgetItem(mProcessList.at(i)));
  }
}


//------------------------------------------------------------------------------------//

void ProcessMonitor::checkProcessList()
{
  if (systemUtil.isBinaryRunning(std::string("syncthing")))
  {
    if (isPausingProcessRunning())
    {
      mpSyncConnector->shutdownSyncthingProcess();
    }
  }
}

//------------------------------------------------------------------------------------//

void ProcessMonitor::addButtonClicked()
{
  if (!mProcessList.contains(mpProcessLineEdit->text()))
  {
    mProcessList.append(mpProcessLineEdit->text());
    saveSettings();
    refreshTable();
  }
}


//------------------------------------------------------------------------------------//

void ProcessMonitor::deleteButtonClicked()
{
  for (QTableWidgetItem *selected : mpProcessTable->selectedItems())
  {
    mProcessList.removeOne(selected->text());
  }
  saveSettings();
  refreshTable();
}


//------------------------------------------------------------------------------------//

void ProcessMonitor::cellSelected(int nRow, int nColumn)
{
UNUSED(nColumn);
  mProcessList.removeAt(nRow);
  saveSettings();
  refreshTable();
}


//------------------------------------------------------------------------------------//

void ProcessMonitor::loadSettings()
{
  mProcessList = mSettings.value("processList").toStringList();
}


//------------------------------------------------------------------------------------//

void ProcessMonitor::saveSettings()
{
  mSettings.setValue("processList", mProcessList);
}
