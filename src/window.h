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

#ifndef WINDOW_H
#define WINDOW_H

#include "syncconnector.h"
#include "processmonitor.hpp"
#include "startuptab.hpp"
#include "platforms.hpp"
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QProcess>
#include <QFileDialog>
#include <QTabWidget>
#include <QMovie>
#include <memory>


#ifndef QT_NO_SYSTEMTRAYICON

#include <QDialog>

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

//! [0]
class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

    void setVisible(bool visible) Q_DECL_OVERRIDE;
    void updateConnectionHealth(ConnectionHealthStatus status);
    void onNetworkActivity(bool activity);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void setIcon(int index);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWebView();
    void messageClicked();
    void testURL();
    void authCheckBoxChanged(int state);
    void monoChromeIconChanged(int state);
    void animateIconBoxChanged(int state);
    void showAboutPage();
    void folderClicked();
    void syncedFileClicked();
    void onUpdateIcon();
    void quit();

private:
    void createSettingsGroupBox();
    void createActions();
    void createTrayIcon();
    void saveSettings();
    void loadSettings();
    void showAuthentication(bool show);
    void showMessage(std::string title, std::string body,
      QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    void createFoldersMenu();
    void createLastSyncedMenu();
    void createDefaultSettings();
    void validateSSLSupport();
    int getCurrentVersion(std::string reply);
    void onStartAnimation(bool animate);

    QTabWidget *mpSettingsTabsWidget;
    QGroupBox *mpSettingsGroupBox;
    QLabel *mpURLLabel;
    QLineEdit *mpSyncthingUrlLineEdit;

    QLabel *userNameLabel;
    QLabel *userPasswordLabel;
    QLineEdit *mpUserNameLineEdit;
    QLineEdit *userPassword;
    QCheckBox *mpAuthCheckBox;

    QLabel *mpUrlTestResultLabel;
    QPushButton *mpTestConnectionButton;
  
    QGroupBox *mpAppearanceGroupBox;
    QCheckBox *mpMonochromeIconBox;
    QCheckBox *mpNotificationsIconBox;
    QCheckBox *mpShouldAnimateIconBox;

    QAction *mpConnectedState;
    QAction *mpNumberOfConnectionsAction;
    QAction *mpCurrentTrafficAction;
    QAction *mpTrafficInAction;
    QAction *mpTrafficOutAction;
    QAction *mpShowWebViewAction;
    QAction *mpPreferencesAction;
    QAction *mpShowGitHubAction;
    QAction *mpQuitAction;

    std::list<QSharedPointer<QAction>> mCurrentFoldersActions;
    std::list<QSharedPointer<QAction>> mCurrentSyncedFilesActions;

    std::list<FolderNameFullPath> mCurrentFoldersLocations;
    LastSyncedFileList mLastSyncedFiles;

    QSystemTrayIcon *mpTrayIcon = nullptr;
    QMenu *mpTrayIconMenu = nullptr;
    QUrl mCurrentUrl;

    std::string mCurrentUserName;
    std::string mCurrentUserPassword;
    std::shared_ptr<qst::connector::SyncConnector> mpSyncConnector;
    std::unique_ptr<qst::monitor::ProcessMonitor> mpProcessMonitor;
    std::unique_ptr<qst::settings::StartupTab> mpStartupTab;
    QSettings mSettings;
  
    std::unique_ptr<QMovie> mpAnimatedIconMovie;

    bool mIconMonochrome;
    bool mNotificationsEnabled;
    bool mShouldAnimateIcon;
    bool mShouldStopAnimation;
    int mLastConnectionState;

  };
//! [0]

#endif // QT_NO_SYSTEMTRAYICON

#endif
