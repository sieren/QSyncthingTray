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

#include "window.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QtGui>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
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


//! Layout
#define maximumWidth 700
//! [0]
//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
Window::Window()
  : mSyncConnector(new mfk::connector::SyncConnector(QUrl(tr("http://127.0.0.1:8384"))))
  , settings("sieren", "QSyncthingTray")
{
    loadSettings();
    createSettingsGroupBox();

    createActions();
    createTrayIcon();

    connect(testConnection, SIGNAL(clicked()), this, SLOT(testURL()));
    connect(syncThingUrl, SIGNAL(currentIndexChanged(int)), this, SLOT(setIcon(int)));
    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
      this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    connect(authCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(authCheckBoxChanged(int)));
    connect(filePathBrowse, SIGNAL(clicked()), this, SLOT(showFileBrowser()));
    connect(filePathLine, SIGNAL(returnPressed()), this, SLOT(pathEnterPressed()));
  
    mpSettingsTabsWidget = new QTabWidget;
  
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    QWidget *settingsPageWidget = new QWidget;
    settingsLayout->addWidget(settingsGroupBox);
    settingsLayout->addWidget(filePathGroupBox);
    settingsPageWidget->setLayout(settingsLayout);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mpSettingsTabsWidget->addTab(settingsPageWidget, "Main");
    mainLayout->addWidget(mpSettingsTabsWidget);
    setLayout(mainLayout);
    testURL();
    mSyncConnector->setConnectionHealthCallback(std::bind(
      &Window::updateConnectionHealth,
      this,
      std::placeholders::_1));

    mSyncConnector->setProcessSpawnedCallback([&](kSyncthingProcessState state)
      {
        switch (state) {
          case kSyncthingProcessState::SPAWNED:
            appSpawnedLabel->setText(tr("Launched"));
            break;
          case kSyncthingProcessState::NOT_RUNNING:
            appSpawnedLabel->setText(tr("Not started"));
            break;
          case kSyncthingProcessState::ALREADY_RUNNING:
            appSpawnedLabel->setText(tr("Already Runnning"));
            break;
          default:
            break;
        }
      });

    mSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath);

    setIcon(0);
    trayIcon->show();
    #ifdef Q_OS_MAC
      this->setWindowIcon(QIcon(":/images/syncthing.icns"));
    #endif
    setWindowTitle(tr("QSyncthingTray"));
    resize(maximumWidth / devicePixelRatio(), 400);
}


//------------------------------------------------------------------------------------//

void Window::setVisible(bool visible)
{
  QDialog::setVisible(visible);
  raise();
}


//------------------------------------------------------------------------------------//

void Window::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible())
    {
        hide();
        if (filePathLine->text().toStdString() != mCurrentSyncthingPath)
        {
          mCurrentSyncthingPath = filePathLine->text().toStdString();
          saveSettings();
          spawnSyncthingApp();
        }
        event->ignore();
    }
}


//------------------------------------------------------------------------------------//

void Window::setIcon(int index)
{
  QIcon icon;
  switch(index)
  {
    case 0:
      icon = QIcon(":/images/syncthingBlue.png");
      break;
    case 1:
      icon = QIcon(":/images/syncthingGrey.png");
      break;
    default:
      icon = QIcon(":/images/syncthingGrey.png");
      break;
  }
  trayIcon->setIcon(icon);
  setWindowIcon(icon);

  trayIcon->setToolTip("Syncthing");
}


//------------------------------------------------------------------------------------//

void Window::testURL()
{
  mCurrentUrl = QUrl(syncThingUrl->text());
  mCurrentUserName = userName->text().toStdString();
  mCurrentUserPassword = userPassword->text().toStdString();
  mSyncConnector->setURL(QUrl(syncThingUrl->text()), mCurrentUserName,
    mCurrentUserPassword, [&](std::string result, bool success)
  {
    if (success)
    {
      urlTestResultLabel->setText(tr("Connected"));
      connectedState->setText(tr("Connected"));
      setIcon(0);
    }
    else
    {
      urlTestResultLabel->setText(tr("Error: ") + result.c_str());
      setIcon(1);
    }
  });
  saveSettings();
}


//------------------------------------------------------------------------------------//

void Window::updateConnectionHealth(std::map<std::string, std::string> status)
{
  if (status.at("state") == "1")
  {
    std::string connectionNumber = status.at("connections");
    numberOfConnectionsAction->setVisible(true);
    numberOfConnectionsAction->setText(tr("Connections: ") + connectionNumber.c_str());
    connectedState->setText(tr("Connected"));
    setIcon(0);
    if (lastState != 1)
    {
      showMessage("Connected", "Syncthing is running.");
    }
  }
  else
  {
    connectedState->setText(tr("Not Connected"));
    if (lastState != 0)
    {
      showMessage("Not Connected", "Could not find Syncthing.");
    }
    // syncthing takes a while to shut down, in case someone
    // would reopen qsyncthingtray it wouldnt restart the process
    mSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath);
    setIcon(1);
  }
  try
  {
    lastState = std::stoi(status.at("state"));
  }
  catch (std::exception &e)
  {
    
  }
  createFoldersMenu();
}


//------------------------------------------------------------------------------------//

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason)
  {
  case QSystemTrayIcon::Trigger:
  case QSystemTrayIcon::DoubleClick:
      break;
  case QSystemTrayIcon::MiddleClick:
      break;
  default:
      ;
  }
  setIcon(1);
}


//------------------------------------------------------------------------------------//

void Window::showWebView()
{
  mSyncConnector->showWebView();
}


//------------------------------------------------------------------------------------//

void Window::authCheckBoxChanged(int state)
{
  if (state)
  {
    showAuthentication(true);
  }
  else
  {
    showAuthentication(false);
  }
}


//------------------------------------------------------------------------------------//

void Window::showMessage(std::string title, std::string body)
{
  trayIcon->showMessage(tr(title.c_str()), tr(body.c_str()), QSystemTrayIcon::Warning,
                        1000);
}


//------------------------------------------------------------------------------------//

void Window::showFileBrowser()
{
  QString filename = QFileDialog::getOpenFileName(this,
                                          tr("Open Syncthing"), "", tr(""));
  mCurrentSyncthingPath = filename.toStdString();
  filePathLine->setText(filename);
  spawnSyncthingApp();
}


//------------------------------------------------------------------------------------//
void Window::pathEnterPressed()
{
    mCurrentSyncthingPath = filePathLine->text().toStdString();
    spawnSyncthingApp();
}

//------------------------------------------------------------------------------------//

void Window::spawnSyncthingApp()
{
  saveSettings();
  mSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath);
}


//------------------------------------------------------------------------------------//

void Window::messageClicked()
{
  QDialog::setVisible(true);
  raise();
}


//------------------------------------------------------------------------------------//

void Window::folderClicked()
{
  QObject *obj = sender();
  QAction * senderObject = static_cast<QAction*>(obj);
  std::string findFolder = senderObject->text().toStdString();
  std::list<std::pair<std::string, std::string>>::iterator folder =
    std::find_if(mCurrentFoldersLocations.begin(), mCurrentFoldersLocations.end(),
      [&findFolder](std::pair<std::string, std::string> const& elem) {
        return elem.first == findFolder;
      });
  QDesktopServices::openUrl(QUrl::fromLocalFile(tr(folder->second.c_str())));
}


//------------------------------------------------------------------------------------//

void Window::createSettingsGroupBox()
{
  
  settingsGroupBox = new QGroupBox(tr("Syncthing URL"));

  iconLabel = new QLabel("URL");

  syncThingUrl = new QLineEdit(mCurrentUrl.toString());
  syncThingUrl->setFixedWidth(maximumWidth / devicePixelRatio());
  testConnection = new QPushButton(tr("Connect"));

  authCheckBox = new QCheckBox(tr("Authentication"), this);

  userNameLabel = new QLabel("User");
  userPasswordLabel = new QLabel("Password");

  userName = new QLineEdit(mCurrentUserName.c_str());
  userPassword = new QLineEdit(mCurrentUserPassword.c_str());
  userPassword->setEchoMode(QLineEdit::Password);

  urlTestResultLabel = new QLabel("Not Tested");

  QGridLayout *iconLayout = new QGridLayout;
  iconLayout->addWidget(iconLabel, 0, 0);
  iconLayout->addWidget(syncThingUrl,1, 0, 1, 4);
  iconLayout->addWidget(authCheckBox, 2, 0, 1, 2);
  iconLayout->addWidget(userNameLabel, 3, 0, 1, 2);
  iconLayout->addWidget(userPasswordLabel, 3, 2, 1 ,2);
  iconLayout->addWidget(userName, 4, 0, 1, 2);
  iconLayout->addWidget(userPassword, 4, 2, 1, 2 );
  iconLayout->addWidget(testConnection,5, 0, 1, 1);
  iconLayout->addWidget(urlTestResultLabel, 5, 1, 1, 2);
  settingsGroupBox->setLayout(iconLayout);
  settingsGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  filePathGroupBox = new QGroupBox(tr("Syncthing Application"));

  filePathLabel = new QLabel("Binary with Path");

  filePathLine = new QLineEdit(mCurrentSyncthingPath.c_str());
  filePathLine->setFixedWidth(maximumWidth / devicePixelRatio());
  filePathBrowse = new QPushButton(tr("Browse"));

  appSpawnedLabel = new QLabel(tr("Not started"));

  authCheckBox->setCheckState(Qt::Checked);
  if (mCurrentUserName.length() == 0)
  {
    showAuthentication(false);
    authCheckBox->setCheckState(Qt::Unchecked);
  }

  QGridLayout *filePathLayout = new QGridLayout;
  filePathLayout->addWidget(filePathLabel, 0, 0);
  filePathLayout->addWidget(filePathLine,1, 0, 1, 4);
  filePathLayout->addWidget(filePathBrowse,2, 0, 1, 1);
  filePathLayout->addWidget(appSpawnedLabel, 2, 1, 1, 1);

  filePathGroupBox->setLayout(filePathLayout);
  filePathGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}


//------------------------------------------------------------------------------------//

void Window::createActions()
{
  connectedState = new QAction(tr("Not Connected"), this);
  connectedState->setDisabled(true);

  numberOfConnectionsAction = new QAction(tr("Connections: 0"), this);
  numberOfConnectionsAction->setDisabled(true);

  showWebViewAction = new QAction(tr("Open Syncthing"), this);
  connect(showWebViewAction, SIGNAL(triggered()), this, SLOT(showWebView()));
      
  preferencesAction = new QAction(tr("Preferences"), this);
  connect(preferencesAction, SIGNAL(triggered()), this, SLOT(showNormal()));
  
  showGitHubAction = new QAction(tr("Help"), this);
  connect(showGitHubAction, SIGNAL(triggered()), this, SLOT(showGitPage()));

  quitAction = new QAction(tr("&Quit"), this);
  connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}


//------------------------------------------------------------------------------------//

void Window::createFoldersMenu()
{
  std::list<QSharedPointer<QAction>> foldersActions;
  if (mCurrentFoldersLocations != mSyncConnector->getFolders())
  {
    std::cout << "Folder List has changed";
    mCurrentFoldersLocations = mSyncConnector->getFolders();
    for (std::list<std::pair<std::string,
      std::string>>::iterator it=mCurrentFoldersLocations.begin();
      it != mCurrentFoldersLocations.end(); ++it)
    {
      QSharedPointer<QAction> aAction = QSharedPointer<QAction>(
        new QAction(tr(it->first.c_str()), this));
      connect(aAction.data(), SIGNAL(triggered()), this, SLOT(folderClicked()));
      foldersActions.emplace_back(aAction);
    }
    mCurrentFoldersActions = foldersActions;
    // Update Menu
    createTrayIcon();
  }
}


//------------------------------------------------------------------------------------//

void Window::createTrayIcon()
{
  if (trayIconMenu == nullptr)
  {
    trayIconMenu = new QMenu(this);
  }
  trayIconMenu->clear();
  trayIconMenu->addAction(connectedState);
  trayIconMenu->addAction(numberOfConnectionsAction);
  trayIconMenu->addSeparator();

  for (std::list<QSharedPointer<QAction>>::iterator it = mCurrentFoldersActions.begin();
      it != mCurrentFoldersActions.end(); ++it)
  {
    QAction *aAction = it->data();
    trayIconMenu->addAction(std::move(aAction));
  }
  
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(showWebViewAction);
  trayIconMenu->addAction(preferencesAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(showGitHubAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);
  if (trayIcon == nullptr)
  {
    trayIcon = new QSystemTrayIcon(this);
  }
  trayIcon->setContextMenu(trayIconMenu);
}


//------------------------------------------------------------------------------------//

void Window::saveSettings()
{
  settings.setValue("url", mCurrentUrl.toString());
  settings.setValue("username", userName->text());
  settings.setValue("userpassword", userPassword->text());
  settings.setValue("syncthingpath", tr(mCurrentSyncthingPath.c_str()));
}


//------------------------------------------------------------------------------------//

void Window::showAuthentication(bool show)
{
  if (show)
  {
    userName->show();
    userPassword->show();
    userNameLabel->show();
    userPasswordLabel->show();
  }
  else
  {
    userName->hide();
    userPassword->hide();
    userNameLabel->hide();
    userPasswordLabel->hide();
  }
}


//------------------------------------------------------------------------------------//

void Window::loadSettings()
{
  mCurrentUrl.setUrl(settings.value("url").toString());
  if (mCurrentUrl.toString().length() == 0)
  {
    mCurrentUrl.setUrl(tr("http://127.0.0.1:8384"));
  }
  mCurrentUserPassword = settings.value("userpassword").toString().toStdString();
  mCurrentUserName = settings.value("username").toString().toStdString();
  mCurrentSyncthingPath = settings.value("syncthingpath").toString().toStdString();
}


//------------------------------------------------------------------------------------//

void Window::showGitPage()
{
  QString link = "http://www.github.com/sieren/QSyncthingTray";
  QDesktopServices::openUrl(QUrl(link));
}

#endif
