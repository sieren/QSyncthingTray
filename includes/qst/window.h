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
#include <qst/settingsmigrator.hpp>
#include <qst/updatenotifier.h>
#include <QDoubleSpinBox>
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

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void updateConnectionHealth(const ConnectionStateData& state);
    void onNetworkActivity(bool activity);
    void setIcon(int index);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWebView();
    void messageClicked();
    void testURL();
    void authCheckBoxChanged(int state);
    void monoChromeIconChanged(int state);
    void animateIconBoxChanged(int state);
    void webViewZoomFactorChanged(double value);
    void showAboutPage();
    void folderClicked();
    void syncedFileClicked();
    void onUpdateIcon();
    void pauseSyncthingClicked(int state);
    void quit();
    void onUpdateConnState(const ConnectionState& result);
    void checkForUpdate();
private:
    qst::settings::SettingsMigrator mSettingsMigrator;
    void createSettingsGroupBox();
    void createActions();
    void createTrayIcon();
    void saveSettings();
    void loadSettings();
    void showAuthentication(bool show);
    void showMessage(const std::string& title, const std::string& body,
      QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    void createFoldersMenu();
    void createLastSyncedMenu();
    void createDefaultSettings();
    void validateSSLSupport();
    void onStartAnimation(bool animate);

    QTabWidget *mpSettingsTabsWidget;
    QGroupBox *mpSettingsGroupBox;
    QLabel *mpURLLabel;
    QLineEdit *mpSyncthingUrlLineEdit;

    QLabel *userNameLabel;
    QLabel *userPasswordLabel;
    QLabel *mpAPIKeyLabel;
    QLineEdit *mpUserNameLineEdit;
    QLineEdit *userPassword;
    QLineEdit *mpAPIKeyEdit;
    QCheckBox *mpAuthCheckBox;

    QLabel *mpUrlTestResultLabel;
    QPushButton *mpTestConnectionButton;
  
    QGroupBox *mpAppearanceGroupBox;
    QCheckBox *mpMonochromeIconBox;
    QCheckBox *mpNotificationsIconBox;
    QCheckBox *mpShouldAnimateIconBox;
    QLabel *mpWebViewZoomFactorLabel;
    QDoubleSpinBox *mpWebViewZoomFactor;
    QLabel *mpSyncPollIntervalLabel;
    QDoubleSpinBox *mpSyncPollIntervalBox;

    QAction *mpConnectedState;
    QAction *mpNumberOfConnectionsAction;
    QAction *mpCurrentTrafficAction;
    QAction *mpTrafficInAction;
    QAction *mpTrafficOutAction;
    QAction *mpShowWebViewAction;
    QAction *mpPreferencesAction;
    QAction *mpShowGitHubAction;
    QAction *mpCheckUpdateAction;
    QAction *mpPauseSyncthingAction;
    QAction *mpQuitAction;

    QList<QAction*> mCurrentFoldersActions;
    QMenu *mpFolderMenu = nullptr;
    QList<QAction*> mCurrentSyncedFilesActions;
    QMenu *mpLastSyncedMenu = nullptr;

    std::list<FolderNameFullPath> mCurrentFoldersLocations;
    LastSyncedFileList mLastSyncedFiles;

    QSystemTrayIcon *mpTrayIcon = nullptr;
    QMenu *mpTrayIconMenu = nullptr;
    QUrl mCurrentUrl;
    int mLastIconIndex = -1;

    QString mCurrentUserName;
    QString mCurrentUserPassword;
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

    qst::update::UpdateNotifier mUpdateNotifier;
    void onUpdateCheck(const bool isNewVersion);

    template <typename T>
    void checkAndSetValue(QString key, T value);

    template <typename U, typename T>
    void setElements(U &&func, T uiElement);

    template <typename U, typename T, typename ... TArgs>
    void setElements(U &&func, T uiElement, TArgs...   Elements);
 
  };
//! [0]

#endif // QT_NO_SYSTEMTRAYICON

#endif
