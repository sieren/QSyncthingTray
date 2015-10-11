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
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QWebView>
#include <QProcess>
#include <QFileDialog>
#include <QTabWidget>
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
    void updateConnectionHealth(std::map<std::string, std::string> status);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void setIcon(int index);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWebView();
    void messageClicked();
    void testURL();
    void authCheckBoxChanged(int state);
    void spawnSyncthingApp();
    void showFileBrowser();
    void showGitPage();
    void folderClicked();
    void pathEnterPressed();

private:
    void createSettingsGroupBox();
    void createActions();
    void createTrayIcon();
    void saveSettings();
    void loadSettings();
    void showAuthentication(bool show);
    void showMessage(std::string title, std::string body);
    void createFoldersMenu();

    QTabWidget *mpSettingsTabsWidget;
    QGroupBox *settingsGroupBox;
    QLabel *iconLabel;
    QLineEdit *syncThingUrl;
    QLabel *userNameLabel;
    QLabel *userPasswordLabel;
    QLineEdit *userName;
    QLineEdit *userPassword;
    QCheckBox *showIconCheckBox;
    QCheckBox *authCheckBox;
  
    QGroupBox *filePathGroupBox;
    QLabel *filePathLabel;
    QLineEdit *filePathLine;
    QPushButton *filePathBrowse;

    QGroupBox *messageGroupBox;
    QLabel *urlTestResultLabel;
    QLabel *appSpawnedLabel;
    QPushButton *showMessageButton;
    QPushButton *testConnection;

    QAction *connectedState;
    QAction *numberOfConnectionsAction;
    QAction *showWebViewAction;
    QAction *preferencesAction;
    QAction *showGitHubAction;
    QAction *quitAction;
    QAction *spawnSyncthingAppAction;

    std::list<QSharedPointer<QAction>> mCurrentFoldersActions;
    std::list<std::pair<std::string, std::string>> mCurrentFoldersLocations;
    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayIconMenu = nullptr;
    QUrl mCurrentUrl;
    QProcess *syncThingApp;
    std::string mCurrentUserName;
    std::string mCurrentUserPassword;
    std::string mCurrentSyncthingPath;
    QNetworkAccessManager m_WebCtrl;
    std::unique_ptr<mfk::connector::SyncConnector> mSyncConnector;
    QSettings settings;
  
    int lastState;

  };
//! [0]

#endif // QT_NO_SYSTEMTRAYICON

#endif
