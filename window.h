/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
    void spawnSyncThingApp();
    void showFileBrowser();
    void showGitPage();

private:
    void createSettingsGroupBox();
    void createActions();
    void createTrayIcon();
    void saveSettings();
    void loadSettings();
    void showAuthentication(bool show);
    void showMessage(std::string title, std::string body);

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
    QAction *spawnSyncThingAppAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QUrl mCurrentUrl;
    QProcess *syncThingApp;
    std::string mCurrentUserName;
    std::string mCurrentUserPassword;
    std::string mCurrentSyncThingPath;
    QNetworkAccessManager m_WebCtrl;
    std::unique_ptr<mfk::connector::SyncConnector> mSyncConnector;
    QSettings settings;

  };
//! [0]

#endif // QT_NO_SYSTEMTRAYICON

#endif
