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
#define maximumWidth 400
static const std::list<std::pair<std::string, std::string>> kIconSet(
{{":/images/syncthingBlue.png", ":/images/syncthingGrey.png"},
{":/images/syncthingBlack.png", ":/images/syncthingGrey.png"}});
//! [0]
//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
Window::Window()
  : mpSyncConnector(new mfk::connector::SyncConnector(QUrl(tr("http://127.0.0.1:8384"))))
  , mpProcessMonitor(new mfk::monitor::ProcessMonitor(mpSyncConnector))
  , mSettings("sieren", "QSyncthingTray")
{
    loadSettings();
    createSettingsGroupBox();

    createActions();
    createTrayIcon();

    connect(mpTestConnectionButton, SIGNAL(clicked()), this, SLOT(testURL()));
    connect(mpTrayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
    connect(mpTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
      this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    connect(mpAuthCheckBox, SIGNAL(stateChanged(int)), this,
      SLOT(authCheckBoxChanged(int)));
    connect(mpMonochromeIconBox, SIGNAL(stateChanged(int)), this,
      SLOT(monoChromeIconChanged(int)));
    connect(mpFilePathBrowse, SIGNAL(clicked()), this, SLOT(showFileBrowser()));
    connect(mpFilePathLine, SIGNAL(returnPressed()), this, SLOT(pathEnterPressed()));

    mpSettingsTabsWidget = new QTabWidget;
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    QWidget *settingsPageWidget = new QWidget;
    settingsLayout->addWidget(mpSettingsGroupBox);
    settingsLayout->addWidget(mpFilePathGroupBox);
    settingsLayout->addWidget(mpAppearanceGroupBox);
    settingsPageWidget->setLayout(settingsLayout);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mpSettingsTabsWidget->addTab(settingsPageWidget, "Main");
    mpSettingsTabsWidget->addTab(mpProcessMonitor.get(), "Auto-Pause");
    mainLayout->addWidget(mpSettingsTabsWidget);
    setLayout(mainLayout);
    testURL();
    mpSyncConnector->setConnectionHealthCallback(std::bind(
      &Window::updateConnectionHealth,
      this,
      std::placeholders::_1));

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

    mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath);

    setIcon(0);
    mpTrayIcon->show();
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
    if (mpTrayIcon->isVisible())
    {
        hide();
        if (mpFilePathLine->text().toStdString() != mCurrentSyncthingPath)
        {
          mCurrentSyncthingPath = mpFilePathLine->text().toStdString();
          spawnSyncthingApp();
        }
        event->ignore();
    }
  saveSettings();
}


//------------------------------------------------------------------------------------//

void Window::setIcon(int index)
{
  QIcon icon;
  std::pair<std::string, std::string> iconSet = mIconMonochrome ?
    kIconSet.back() : kIconSet.front();
  switch(index)
  {
    case 0:
      icon = QIcon(iconSet.first.c_str());
      break;
    case 1:
      icon = QIcon(iconSet.second.c_str());
      break;
    default:
      icon = QIcon(iconSet.second.c_str());
      break;
  }
  mpTrayIcon->setIcon(icon);
  setWindowIcon(icon);

  mpTrayIcon->setToolTip("Syncthing");
}


//------------------------------------------------------------------------------------//

void Window::testURL()
{
  mCurrentUrl = QUrl(mpSyncthingUrlLineEdit->text());
  mCurrentUserName = mpUserNameLineEdit->text().toStdString();
  mCurrentUserPassword = userPassword->text().toStdString();
  mpSyncConnector->setURL(QUrl(mpSyncthingUrlLineEdit->text()), mCurrentUserName,
     mCurrentUserPassword, [&](std::pair<std::string, bool> result)
  {
    if (result.second)
    {
      mpUrlTestResultLabel->setText(tr("Status: Connected"));
      mpConnectedState->setText(tr("Connected"));
      setIcon(0);
    }
    else
    {
      mpUrlTestResultLabel->setText(tr("Status: ") + result.first.c_str());
      setIcon(1);
    }
  });
  saveSettings();
}


//------------------------------------------------------------------------------------//

void Window::updateConnectionHealth(std::map<std::string, std::string> status)
{
  if (mpProcessMonitor->isPausingProcessRunning())
  {
    mpNumberOfConnectionsAction->setVisible(false);
    mpConnectedState->setText(tr("Paused"));
    if (mLastConnectionState != 99)
    {
      showMessage("Paused", "Syncthing is pausing.");
      setIcon(1);
      mLastConnectionState = 99;
    }
    return;
  }
  else if (status.at("state") == "1")
  {
    std::string activeConnections = status.at("activeConnections");
    std::string totalConnections = status.at("totalConnections");
    mpNumberOfConnectionsAction->setVisible(true);
    mpNumberOfConnectionsAction->setText(tr("Connections: ") + activeConnections.c_str()
      + "/" + totalConnections.c_str());
    mpConnectedState->setText(tr("Connected"));
    setIcon(0);
    if (mLastConnectionState != 1)
    {
      showMessage("Connected", "Syncthing is running.");
    }
  }
  else
  {
    mpConnectedState->setText(tr("Not Connected"));
    if (mLastConnectionState != 0)
    {
      showMessage("Not Connected", "Could not find Syncthing.");
    }
    // syncthing takes a while to shut down, in case someone
    // would reopen qsyncthingtray it wouldnt restart the process
    mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath);
    setIcon(1);
  }
  try
  {
    mLastConnectionState = std::stoi(status.at("state"));
  }
  catch (std::exception &e)
  {

  }
  createFoldersMenu();
}

//------------------------------------------------------------------------------------//

void Window::monoChromeIconChanged(int state)
{
  mIconMonochrome = state == 2 ? true : false;
  mSettings.setValue("monochromeIcon", mIconMonochrome);
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
}


//------------------------------------------------------------------------------------//

void Window::showWebView()
{
  mpSyncConnector->showWebView();
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
  if (mNotificationsEnabled)
  {
    mpTrayIcon->showMessage(tr(title.c_str()), tr(body.c_str()), QSystemTrayIcon::Warning,
      1000);
  }
}


//------------------------------------------------------------------------------------//

void Window::showFileBrowser()
{
  QString filename = QFileDialog::getOpenFileName(this,
                                          tr("Open Syncthing"), "", tr(""));
  mCurrentSyncthingPath = filename.toStdString();
  mpFilePathLine->setText(filename);
  saveSettings();
  spawnSyncthingApp();
}


//------------------------------------------------------------------------------------//
void Window::pathEnterPressed()
{
    mCurrentSyncthingPath = mpFilePathLine->text().toStdString();
    mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath, true);
}

//------------------------------------------------------------------------------------//

void Window::spawnSyncthingApp()
{
  saveSettings();
  mpSyncConnector->spawnSyncthingProcess(mCurrentSyncthingPath);
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
  mpSettingsGroupBox = new QGroupBox(tr("Syncthing URL"));

  mpURLLabel = new QLabel("URL");

  mpSyncthingUrlLineEdit = new QLineEdit(mCurrentUrl.toString());
  // mpSyncthingUrlLineEdit->setFixedWidth(maximumWidth - 40 / devicePixelRatio());
  mpTestConnectionButton = new QPushButton(tr("Connect"));

  mpAuthCheckBox = new QCheckBox(tr("Authentication"), this);

  userNameLabel = new QLabel("User");
  userPasswordLabel = new QLabel("Password");

  mpUserNameLineEdit = new QLineEdit(mCurrentUserName.c_str());
  userPassword = new QLineEdit(mCurrentUserPassword.c_str());
  userPassword->setEchoMode(QLineEdit::Password);

  mpUrlTestResultLabel = new QLabel("Not Tested");

  QGridLayout *iconLayout = new QGridLayout;
  iconLayout->addWidget(mpURLLabel, 0, 0);
  iconLayout->addWidget(mpSyncthingUrlLineEdit,1, 0, 1, 4);
  iconLayout->addWidget(mpAuthCheckBox, 2, 0, 1, 2);
  iconLayout->addWidget(userNameLabel, 3, 0, 1, 2);
  iconLayout->addWidget(userPasswordLabel, 3, 2, 1 ,2);
  iconLayout->addWidget(mpUserNameLineEdit, 4, 0, 1, 2);
  iconLayout->addWidget(userPassword, 4, 2, 1, 2 );
  iconLayout->addWidget(mpTestConnectionButton,5, 0, 1, 1);
  iconLayout->addWidget(mpUrlTestResultLabel, 5, 1, 1, 2);
  mpSettingsGroupBox->setLayout(iconLayout);
  mpSettingsGroupBox->setMinimumWidth(400);
  mpSettingsGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  mpFilePathGroupBox = new QGroupBox(tr("Syncthing Application"));

  mpFilePathLabel = new QLabel("Binary with Path");

  mpFilePathLine = new QLineEdit(mCurrentSyncthingPath.c_str());
//  mpFilePathLine->setFixedWidth(maximumWidth / devicePixelRatio());
  mpFilePathBrowse = new QPushButton(tr("Browse"));

  mpAppSpawnedLabel = new QLabel(tr("Not started"));

  mpAuthCheckBox->setCheckState(Qt::Checked);
  if (mCurrentUserName.length() == 0)
  {
    showAuthentication(false);
    mpAuthCheckBox->setCheckState(Qt::Unchecked);
  }

  QGridLayout *filePathLayout = new QGridLayout;
  filePathLayout->addWidget(mpFilePathLabel, 0, 0);
  filePathLayout->addWidget(mpFilePathLine,1, 0, 1, 4);
  filePathLayout->addWidget(mpFilePathBrowse,2, 0, 1, 1);
  filePathLayout->addWidget(mpAppSpawnedLabel, 2, 1, 1, 1);

  mpFilePathGroupBox->setLayout(filePathLayout);
  mpFilePathGroupBox->setMinimumWidth(400);
  mpFilePathGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  mpAppearanceGroupBox = new QGroupBox(tr("Appearance"));
  mpMonochromeIconBox = new QCheckBox("Monochrome Icon");
  mpMonochromeIconBox->setChecked(mIconMonochrome);
  mpNotificationsIconBox = new QCheckBox("Notifications");
  mpNotificationsIconBox->setChecked(mNotificationsEnabled);
  QGridLayout *appearanceLayout = new QGridLayout;
  appearanceLayout->addWidget(mpMonochromeIconBox, 0, 0);
  appearanceLayout->addWidget(mpNotificationsIconBox, 1, 0);
  mpAppearanceGroupBox->setLayout(appearanceLayout);
  mpAppearanceGroupBox->setMinimumWidth(400);
  mpAppearanceGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}


//------------------------------------------------------------------------------------//

void Window::createActions()
{
  mpConnectedState = new QAction(tr("Not Connected"), this);
  mpConnectedState->setDisabled(true);

  mpNumberOfConnectionsAction = new QAction(tr("Connections: 0"), this);
  mpNumberOfConnectionsAction->setDisabled(true);

  mpShowWebViewAction = new QAction(tr("Open Syncthing"), this);
  connect(mpShowWebViewAction, SIGNAL(triggered()), this, SLOT(showWebView()));

  mpPreferencesAction = new QAction(tr("Preferences"), this);
  connect(mpPreferencesAction, SIGNAL(triggered()), this, SLOT(showNormal()));
  connect(mpPreferencesAction, SIGNAL(closeEvent()), this, SLOT(closePrefs()));

  mpShowGitHubAction = new QAction(tr("Help"), this);
  connect(mpShowGitHubAction, SIGNAL(triggered()), this, SLOT(showGitPage()));

  mpQuitAction = new QAction(tr("&Quit"), this);
  connect(mpQuitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}


//------------------------------------------------------------------------------------//

void Window::createFoldersMenu()
{
  std::list<QSharedPointer<QAction>> foldersActions;
  if (mCurrentFoldersLocations != mpSyncConnector->getFolders())
  {
    std::cout << "Folder List has changed";
    mCurrentFoldersLocations = mpSyncConnector->getFolders();
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
  if (mpTrayIconMenu == nullptr)
  {
    mpTrayIconMenu = new QMenu(this);
  }
  mpTrayIconMenu->clear();
  mpTrayIconMenu->addAction(mpConnectedState);
  mpTrayIconMenu->addAction(mpNumberOfConnectionsAction);
  mpTrayIconMenu->addSeparator();

  for (std::list<QSharedPointer<QAction>>::iterator it = mCurrentFoldersActions.begin();
      it != mCurrentFoldersActions.end(); ++it)
  {
    QAction *aAction = it->data();
    mpTrayIconMenu->addAction(std::move(aAction));
  }

  mpTrayIconMenu->addSeparator();
  mpTrayIconMenu->addAction(mpShowWebViewAction);
  mpTrayIconMenu->addAction(mpPreferencesAction);
  mpTrayIconMenu->addSeparator();
  mpTrayIconMenu->addAction(mpShowGitHubAction);
  mpTrayIconMenu->addSeparator();
  mpTrayIconMenu->addAction(mpQuitAction);
  if (mpTrayIcon == nullptr)
  {
    mpTrayIcon = new QSystemTrayIcon(this);
  }
  mpTrayIcon->setContextMenu(mpTrayIconMenu);
}


//------------------------------------------------------------------------------------//

void Window::saveSettings()
{
  mSettings.setValue("url", mCurrentUrl.toString());
  mSettings.setValue("username", mpUserNameLineEdit->text());
  mSettings.setValue("userpassword", userPassword->text());
  if (mSettings.value("syncthingpath").toString().toStdString() != mCurrentSyncthingPath)
  {
    pathEnterPressed();
  }
  mSettings.setValue("syncthingpath", tr(mCurrentSyncthingPath.c_str()));
  mSettings.setValue("monochromeIcon", mIconMonochrome);
  mSettings.setValue("notificationsEnabled", mNotificationsEnabled);
}


//------------------------------------------------------------------------------------//

void Window::showAuthentication(bool show)
{
  if (show)
  {
    mpUserNameLineEdit->show();
    userPassword->show();
    userNameLabel->show();
    userPasswordLabel->show();
  }
  else
  {
    mpUserNameLineEdit->hide();
    userPassword->hide();
    userNameLabel->hide();
    userPasswordLabel->hide();
  }
}


//------------------------------------------------------------------------------------//

void Window::loadSettings()
{
  if (!mSettings.value("doSettingsExist").toBool())
  {
    createDefaultSettings();
  }

  mCurrentUrl.setUrl(mSettings.value("url").toString());
  if (mCurrentUrl.toString().length() == 0)
  {
    mCurrentUrl.setUrl(tr("http://127.0.0.1:8384"));
  }
  mCurrentUserPassword = mSettings.value("userpassword").toString().toStdString();
  mCurrentUserName = mSettings.value("username").toString().toStdString();
  mCurrentSyncthingPath = mSettings.value("syncthingpath").toString().toStdString();
  mIconMonochrome = mSettings.value("monochromeIcon").toBool();
  mNotificationsEnabled = mSettings.value("notificationsEnabled").toBool();
}


//------------------------------------------------------------------------------------//

void Window::createDefaultSettings()
{
  mSettings.setValue("url", tr("http://127.0.0.1:8384"));
  mSettings.setValue("monochromeIcon", false);
  mSettings.setValue("notificationsEnabled", true);
  mSettings.setValue("doSettingsExist", true);
}

//------------------------------------------------------------------------------------//

void Window::showGitPage()
{
  QString link = "http://www.github.com/sieren/QSyncthingTray";
  QDesktopServices::openUrl(QUrl(link));
}

#endif
