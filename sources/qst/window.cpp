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

#include <qst/window.h>

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
#define currentVersion "v0.5.1"
#define maximumWidth 400
static const std::list<std::pair<std::string, std::string>> kIconSet(
  {{":/images/syncthingBlue.png", ":/images/syncthingGrey.png"},
  {":/images/syncthingBlack.png", ":/images/syncthingGrey.png"}});
static const std::list<std::string> kAnimatedIconSet(
  {":/images/syncthingBlueAnim.gif",
  ":/images/syncthingBlackAnim.gif"});
//! [0]
//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
Window::Window()
  : mpSyncConnector(new qst::connector::SyncConnector(QUrl(tr("http://127.0.0.1:8384"))))
  , mpProcessMonitor(new qst::monitor::ProcessMonitor(mpSyncConnector))
  , mpStartupTab(new qst::settings::StartupTab(mpSyncConnector))
  , mSettings("QSyncthingTray", "qst")
  , mpAnimatedIconMovie(new QMovie())
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
    connect(mpShouldAnimateIconBox, SIGNAL(stateChanged(int)), this,
      SLOT(animateIconBoxChanged(int)));
    connect(mpAnimatedIconMovie.get(), SIGNAL(frameChanged(int)),
      this, SLOT(onUpdateIcon()));
    connect(mpWebViewZoomFactor, SIGNAL(valueChanged(double)), this,
      SLOT(webViewZoomFactorChanged(double)));

    // Setup SyncthingConnector
    using namespace qst::connector;
    connect(mpSyncConnector.get(), &SyncConnector::onConnectionHealthChanged, this,
      &Window::updateConnectionHealth);
    connect(mpSyncConnector.get(), &SyncConnector::onNetworkActivityChanged, this,
          &Window::onNetworkActivity);


    mpSettingsTabsWidget = new QTabWidget;
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->setAlignment(Qt::AlignTop);
    QWidget *settingsPageWidget = new QWidget;
    settingsLayout->addWidget(mpSettingsGroupBox);
   // settingsLayout->addWidget(mpFilePathGroupBox);
    settingsLayout->addWidget(mpAppearanceGroupBox);
    settingsPageWidget->setLayout(settingsLayout);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mpSettingsTabsWidget->addTab(settingsPageWidget, "Main");
    mpSettingsTabsWidget->addTab(mpStartupTab.get(), "Launcher");
    mpSettingsTabsWidget->addTab(mpProcessMonitor.get(), "Auto-Pause");
    mainLayout->addWidget(mpSettingsTabsWidget);
    setLayout(mainLayout);
    testURL();

    setIcon(0);
    mpTrayIcon->show();
    #ifdef Q_OS_MAC
      this->setWindowIcon(QIcon(":/images/syncthing.icns"));
    #endif
    this->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("QSyncthingTray"));
    resize(maximumWidth / devicePixelRatio(), 400);
}


//------------------------------------------------------------------------------------//

void Window::setVisible(bool visible)
{
  QDialog::setVisible(visible);
  qst::sysutils::SystemUtility().showDockIcon(visible);
  raise();
}


//------------------------------------------------------------------------------------//

void Window::closeEvent(QCloseEvent *event)
{
    if (mpTrayIcon->isVisible())
    {
        hide();
        event->ignore();
    }
  qst::sysutils::SystemUtility().showDockIcon(false);
  mpStartupTab->saveSettings();
  saveSettings();
}


//------------------------------------------------------------------------------------//

void Window::setIcon(int index)
{
  // temporary workaround as setIcon seems to leak memory
  // https://bugreports.qt.io/browse/QTBUG-23658?jql=text%20~%20%22setIcon%20memory%22
  // https://bugreports.qt.io/browse/QTBUG-16113?jql=text%20~%20%22setIcon%20memory%22

  if (index != mLastIconIndex)
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

    if (mpAnimatedIconMovie->state() != QMovie::Running)
    {
      mpTrayIcon->setIcon(icon);
    }
    setWindowIcon(icon);

    mpTrayIcon->setToolTip("Syncthing");
    mLastIconIndex = index;
  }
}


//------------------------------------------------------------------------------------//

void Window::testURL()
{
  saveSettings();
  std::string validateUrl = mpSyncthingUrlLineEdit->text().toStdString();
  std::size_t foundSSL = validateUrl.find("https");
  if (foundSSL!=std::string::npos)
  {
    validateSSLSupport();
  }
  mCurrentUrl = QUrl(mpSyncthingUrlLineEdit->text());
  mCurrentUserName = mpUserNameLineEdit->text().toStdString();
  mCurrentUserPassword = userPassword->text().toStdString();
  mpSyncConnector->setURL(QUrl(mpSyncthingUrlLineEdit->text()), mCurrentUserName,
     mCurrentUserPassword, [&](ConnectionState result)
  {
    if (result.second)
    {
      mpUrlTestResultLabel->setText(tr("Status: Connected"));
      mpConnectedState->setText(tr("Connected"));
      setIcon(0);
    }
    else
    {
      mpUrlTestResultLabel->setText(tr("Status: ") + result.first);
      setIcon(1);
    }
  });
  saveSettings();
}


//------------------------------------------------------------------------------------//

void Window::updateConnectionHealth(ConnectionHealthStatus status)
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
    auto activeConnections = status.at("activeConnections");
    auto totalConnections = status.at("totalConnections");
    mpNumberOfConnectionsAction->setVisible(true);
    mpNumberOfConnectionsAction->setText(tr("Connections: ")
      + activeConnections
      + "/" + totalConnections);
    mpConnectedState->setVisible(true);
    mpConnectedState->setText(tr("Connected"));
    mpCurrentTrafficAction->setVisible(true);
    mpCurrentTrafficAction->setText(tr("Total: ")
      + status.at("globalTraffic"));
    mpTrafficInAction->setVisible(true);
    mpTrafficInAction->setText(tr("In: ") + status.at("inTraffic"));
    mpTrafficOutAction->setVisible(true);
    mpTrafficOutAction->setText(tr("Out: ") + status.at("outTraffic"));
    
    if (mLastSyncedFiles != mpSyncConnector->getLastSyncedFiles())
    {
      mLastSyncedFiles = mpSyncConnector->getLastSyncedFiles();
      createLastSyncedMenu();
    }
    setIcon(0);
    if (mLastConnectionState != 1)
    {
      showMessage("Connected", "Syncthing is running.");
    }
  }
  else
  {
    mpConnectedState->setText(tr("Not Connected"));
    mpTrafficInAction->setVisible(false);
    mpTrafficOutAction->setVisible(false);
    mpCurrentTrafficAction->setVisible(false);
    mpNumberOfConnectionsAction->setVisible(false);
    setIcon(1);
  }
  try
  {
    mLastConnectionState = std::stoi(status.at("state").toStdString());
  }
  catch (std::exception &e)
  {
    std::cerr << "Unable to get current Connection Status!" << std::endl;
  }
  createFoldersMenu();
}


//------------------------------------------------------------------------------------//

void Window::onNetworkActivity(bool activity)
{
  onStartAnimation(activity);
}


//------------------------------------------------------------------------------------//

void Window::monoChromeIconChanged(int state)
{
  mIconMonochrome = state == 2 ? true : false;
  mSettings.setValue("monochromeIcon", mIconMonochrome);
}


//------------------------------------------------------------------------------------//

void Window::pauseSyncthingClicked(int state)
{
  mpSyncConnector->pauseSyncthing(state == 1);
}


//------------------------------------------------------------------------------------//

void Window::animateIconBoxChanged(int state)
{
  mShouldAnimateIcon = state == Qt::CheckState::Checked ? true : false;
  mSettings.setValue("animationEnabled", mShouldAnimateIcon);
}


//------------------------------------------------------------------------------------//

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason)
  {
  case QSystemTrayIcon::Trigger:
  case QSystemTrayIcon::DoubleClick:
    qst::sysutils::SystemUtility().doubleClicked(&Window::showWebView, this);
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

void Window::webViewZoomFactorChanged(double value)
{
  if (mpSyncConnector->getWebView())
  {
    mpSyncConnector->getWebView()->setZoomFactor(value);
  }
   mSettings.setValue("WebZoomFactor", value);
}


//------------------------------------------------------------------------------------//

void Window::showMessage(std::string title, std::string body,
  QSystemTrayIcon::MessageIcon icon)
{
  if (mNotificationsEnabled)
  {
    mpTrayIcon->showMessage(tr(title.c_str()), tr(body.c_str()), icon, 1000);
  }
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
  QString findFolder = senderObject->text();
  std::list<FolderNameFullPath>::iterator folder =
    std::find_if(mCurrentFoldersLocations.begin(), mCurrentFoldersLocations.end(),
      [&findFolder](FolderNameFullPath const& elem) {
        return elem.first == findFolder;
      });
  QDesktopServices::openUrl(QUrl::fromLocalFile(folder->second));
}


//------------------------------------------------------------------------------------//

void Window::syncedFileClicked()
{
  using namespace qst::utilities;
  using namespace qst::sysutils;

  QObject *obj = sender();
  QAction * senderObject = static_cast<QAction*>(obj);
  QString findFile = senderObject->text();
  LastSyncedFileList::iterator fileIterator =
  std::find_if(mLastSyncedFiles.begin(), mLastSyncedFiles.end(),
               [&findFile](DateFolderFile const& elem) {
                 return getCleanFileName(std::get<2>(elem)) == findFile;
               });
  
  // get full path to folder
  std::list<FolderNameFullPath>::iterator folder =
  std::find_if(mCurrentFoldersLocations.begin(), mCurrentFoldersLocations.end(),
               [&fileIterator](FolderNameFullPath const& elem) {
                 return getFullCleanFileName(elem.first) == std::get<1>(*fileIterator);
               });
  std::string fullPath = folder->second.toStdString() + getPathToFileName(std::get<2>(*fileIterator).toStdString())
    + SystemUtility().getPlatformDelimiter();
  QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath.c_str()));
}


//------------------------------------------------------------------------------------//

void Window::createSettingsGroupBox()
{
  
  //
  // SYNCTHING API URL
  //

  mpSettingsGroupBox = new QGroupBox(tr("Syncthing URL"));

  mpURLLabel = new QLabel("URL");

  mpSyncthingUrlLineEdit = new QLineEdit(mCurrentUrl.toString());
  mpTestConnectionButton = new QPushButton(tr("Connect"));

  mpAuthCheckBox = new QCheckBox(tr("Authentication"), this);

  userNameLabel = new QLabel("User");
  userPasswordLabel = new QLabel("Password");

  mpUserNameLineEdit = new QLineEdit(mCurrentUserName.c_str());
  userPassword = new QLineEdit(mCurrentUserPassword.c_str());
  userPassword->setEchoMode(QLineEdit::Password);

  mpAPIKeyLabel = new QLabel("API Key");
  mpAPIKeyEdit = new QLineEdit(mSettings.value("apiKey").toString());

  mpUrlTestResultLabel = new QLabel("Not Tested");

  mpAuthCheckBox->setCheckState(Qt::Checked);
  if (mCurrentUserName.length() == 0)
  {
    showAuthentication(false);
    mpAuthCheckBox->setCheckState(Qt::Unchecked);
  }

  QGridLayout *iconLayout = new QGridLayout;
  iconLayout->addWidget(mpURLLabel, 0, 0);
  iconLayout->addWidget(mpSyncthingUrlLineEdit,1, 0, 1, 4);
  iconLayout->addWidget(mpAuthCheckBox, 2, 0, 1, 2);
  iconLayout->addWidget(userNameLabel, 3, 0, 1, 2);
  iconLayout->addWidget(userPasswordLabel, 3, 2, 1 ,2);
  iconLayout->addWidget(mpUserNameLineEdit, 4, 0, 1, 2);
  iconLayout->addWidget(userPassword, 4, 2, 1, 2 );
  iconLayout->addWidget(mpAPIKeyLabel, 5, 0, 1, 2);
  iconLayout->addWidget(mpAPIKeyEdit, 6, 0, 1, 4);
  iconLayout->addWidget(mpTestConnectionButton,7, 0, 1, 1);
  iconLayout->addWidget(mpUrlTestResultLabel, 7, 1, 1, 2);
  mpSettingsGroupBox->setLayout(iconLayout);
  mpSettingsGroupBox->setMinimumWidth(400);
  mpSettingsGroupBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  

  //
  // APPEARANCE BOX
  //

  mpAppearanceGroupBox = new QGroupBox(tr("Appearance"));
  mpMonochromeIconBox = new QCheckBox("Monochrome Icon");
  mpMonochromeIconBox->setChecked(mIconMonochrome);
  mpNotificationsIconBox = new QCheckBox("Notifications");
  mpNotificationsIconBox->setChecked(mNotificationsEnabled);
  mpShouldAnimateIconBox = new QCheckBox("Animate Icon on Activity");
  mpShouldAnimateIconBox->setChecked(mShouldAnimateIcon);
  mpWebViewZoomFactorLabel = new QLabel(tr("Web View Zoom Factor"));
  mpWebViewZoomFactor = new QDoubleSpinBox();
  mpWebViewZoomFactor->setRange(0.0, 4.0);
  mpWebViewZoomFactor->setSingleStep(0.25);
  mpWebViewZoomFactor->setMaximumWidth(80);
  mpWebViewZoomFactor->setValue(mSettings.value("WebZoomFactor").toDouble());

  mpSyncPollIntervalLabel = new QLabel(tr("Polling Interval"));
  mpSyncPollIntervalBox = new QDoubleSpinBox();
  mpSyncPollIntervalBox->setRange(0.0, 10.0);
  mpSyncPollIntervalBox->setSingleStep(0.5);
  mpSyncPollIntervalBox->setMaximumWidth(80);
  mpSyncPollIntervalBox->setValue(mSettings.value("pollingInterval").toDouble());

  QGridLayout *appearanceLayout = new QGridLayout;
  appearanceLayout->addWidget(mpMonochromeIconBox, 0, 0);
  appearanceLayout->addWidget(mpNotificationsIconBox, 1, 0);
  appearanceLayout->addWidget(mpShouldAnimateIconBox, 2, 0);
  appearanceLayout->addWidget(mpWebViewZoomFactorLabel, 3, 0);
  appearanceLayout->addWidget(mpWebViewZoomFactor, 4, 0, 1, 2);
  appearanceLayout->addWidget(mpSyncPollIntervalLabel, 3, 1);
  appearanceLayout->addWidget(mpSyncPollIntervalBox, 4, 1, 1, 2);
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
  
  mpCurrentTrafficAction = new QAction(tr("Total: 0.00 KB/s"), this);
  mpTrafficInAction = new QAction(tr("In: 0 KB/s"), this);
  mpTrafficOutAction = new QAction(tr("Out: 0 KB/s"), this);

  mpShowWebViewAction = new QAction(tr("Open Syncthing"), this);
  connect(mpShowWebViewAction, SIGNAL(triggered()), this, SLOT(showWebView()));

  mpPauseSyncthingAction = new QAction(tr("Pause Syncthing"), this);
  mpPauseSyncthingAction->setCheckable(true);
  connect(mpPauseSyncthingAction, &QAction::triggered, this,
    &Window::pauseSyncthingClicked);

  mpPreferencesAction = new QAction(tr("Preferences"), this);
  connect(mpPreferencesAction, SIGNAL(triggered()), this, SLOT(showNormal()));

  mpShowGitHubAction = new QAction(tr("About"), this);
  connect(mpShowGitHubAction, SIGNAL(triggered()), this, SLOT(showAboutPage()));

  mpQuitAction = new QAction(tr("&Quit"), this);
  connect(mpQuitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}


//------------------------------------------------------------------------------------//

void Window::quit()
{
  qApp->quit();
}


//------------------------------------------------------------------------------------//

void Window::createFoldersMenu()
{

  if (mCurrentFoldersLocations != mpSyncConnector->getFolders())
  {
    mpFolderMenu->clear();
    for (auto action : mCurrentFoldersActions)
    {
      action->deleteLater();
    }
    mCurrentFoldersActions.clear();
    mCurrentFoldersLocations = mpSyncConnector->getFolders();
    for (std::list<FolderNameFullPath>::iterator it=mCurrentFoldersLocations.begin();
      it != mCurrentFoldersLocations.end(); ++it)
    {
      QAction *aAction = new QAction(it->first, this);
      connect(aAction, SIGNAL(triggered()), this, SLOT(folderClicked()));
      mCurrentFoldersActions.push_back(aAction);
    }
    mpFolderMenu->addActions(mCurrentFoldersActions);
  }
}


//------------------------------------------------------------------------------------//

void Window::createLastSyncedMenu()
{
  using namespace qst::utilities;
  if (mLastSyncedFiles.size() > 0)
  {
    mpLastSyncedMenu->clear();
    for (auto action : mCurrentSyncedFilesActions)
    {
      action->deleteLater();
    }
    mCurrentSyncedFilesActions.clear();
    for (LastSyncedFileList::iterator it=mLastSyncedFiles.begin();
         it != mLastSyncedFiles.end(); ++it)
    {
      QAction *aAction = new QAction(getCleanFileName(std::get<2>(*it)), this);

      // 4th item of tuple is file-erased-bool
      aAction->setDisabled(std::get<3>(*it));
      connect(aAction, SIGNAL(triggered()), this, SLOT(syncedFileClicked()));
      mCurrentSyncedFilesActions.push_back(aAction);
    }
    mpLastSyncedMenu->addActions(mCurrentSyncedFilesActions);
  }
}


//------------------------------------------------------------------------------------//

void Window::createTrayIcon()
{
  if (mpTrayIconMenu == nullptr && mpFolderMenu == nullptr &&
      mpLastSyncedMenu == nullptr)
  {
    mpTrayIconMenu = new QMenu(this);
    mpFolderMenu = new QMenu(tr("Folders"), this);
    mpLastSyncedMenu = new QMenu(tr("Last Synced"), this);
    mpLastSyncedMenu->addAction(tr("None"));
  }
  mpTrayIconMenu->clear();
  mpTrayIconMenu->addAction(mpConnectedState);
  mpTrayIconMenu->addAction(mpNumberOfConnectionsAction);
  mpTrayIconMenu->addAction(mpTrafficInAction);
  mpTrayIconMenu->addAction(mpTrafficOutAction);
  mpTrayIconMenu->addAction(mpCurrentTrafficAction);
  mpTrayIconMenu->addAction(mpPauseSyncthingAction);
  mpTrayIconMenu->addSeparator();

  mpTrayIconMenu->addMenu(mpFolderMenu);
  mpTrayIconMenu->addMenu(mpLastSyncedMenu);
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
  mSettings.setValue("monochromeIcon", mIconMonochrome);
  mNotificationsEnabled = mpNotificationsIconBox->checkState() ==
  Qt::CheckState::Checked ? true : false;
  mSettings.setValue("notificationsEnabled", mNotificationsEnabled);
  mSettings.setValue("animationEnabled", mShouldAnimateIcon);
  mSettings.setValue("pollingInterval", mpSyncPollIntervalBox->value());
  mSettings.setValue("apiKey", mpAPIKeyEdit->text());
  mpSyncConnector->onSettingsChanged();
}


//------------------------------------------------------------------------------------//

void Window::showAuthentication(bool show)
{
  if (show)
  {
    setElements(&QWidget::show, mpUserNameLineEdit, userPassword,userNameLabel,
      userPasswordLabel);
  }
  else
  {
    setElements(&QWidget::hide, mpUserNameLineEdit, userPassword, userNameLabel,
      userPasswordLabel);
  }
}


//------------------------------------------------------------------------------------//

void Window::loadSettings()
{
  mCurrentUrl.setUrl(mSettings.value("url").toString());
  mCurrentUserPassword = mSettings.value("userpassword").toString().toStdString();
  mCurrentUserName = mSettings.value("username").toString().toStdString();
  mIconMonochrome = mSettings.value("monochromeIcon").toBool();
  mNotificationsEnabled = mSettings.value("notificationsEnabled").toBool();
  mShouldAnimateIcon = mSettings.value("animationEnabled").toBool();
}




//------------------------------------------------------------------------------------//

void Window::validateSSLSupport()
{
  if (!QSslSocket::supportsSsl())
  {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("OpenSSL Not Found");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(qst::sysutils::SystemUtility().getSSLLibraryText().c_str());
    msgBox.exec();
  }
}


//------------------------------------------------------------------------------------//

void Window::onStartAnimation(bool animate)
{
  QString iconToAnimate = mIconMonochrome ? tr(kAnimatedIconSet.back().c_str())
    : tr(kAnimatedIconSet.front().c_str());
  if (!animate || !mShouldAnimateIcon)
  {
    mShouldStopAnimation = true;
  }
  else if (animate && mShouldAnimateIcon
    && mpAnimatedIconMovie->state() != QMovie::Running)
  {
    mShouldStopAnimation = false;
    mpAnimatedIconMovie->setFileName(iconToAnimate);
    mpAnimatedIconMovie->start();
  }
}


//------------------------------------------------------------------------------------//

void Window::onUpdateIcon()
{
  if (mShouldStopAnimation && mpAnimatedIconMovie->currentFrameNumber() ==
    mpAnimatedIconMovie->frameCount() - 1)
  {
    mpAnimatedIconMovie->stop();
  }
  mpTrayIcon->setIcon(QIcon(QPixmap::fromImage(mpAnimatedIconMovie->currentImage())));
}


//------------------------------------------------------------------------------------//

void Window::showAboutPage()
{
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("About QSyncthingTray");
  msgBox.setTextFormat(Qt::RichText);
  msgBox.setText("<p align='center'>" currentVersion " (c) 2015 The QSyncthingTray " \
    "Authors. <br/> This program comes with absolutely no warranty. <br/><br/>" \
    "For more visit <a href='http://www.github.com/sieren/qsyncthingtray/'>" \
    "QSyncthingTray on Github</a></p>");
  msgBox.exec();
}


//------------------------------------------------------------------------------------//

template <typename U, typename T, typename ... TArgs>
void Window::setElements(U &&func, T uiElement, TArgs...   Elements)
{
  std::function<void()> functionCall = std::bind(func, uiElement);
  functionCall();
  setElements(func, std::forward<TArgs>(Elements)...);
}


//------------------------------------------------------------------------------------//

template <typename U, typename T>
void Window::setElements(U &&func, T uiElement)
{
  std::function<void()> functionCall = std::bind(func, uiElement);
  functionCall();
}


//------------------------------------------------------------------------------------//

#endif

//------------------------------------------------------------------------------------//
// EOF
//------------------------------------------------------------------------------------//
