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

#ifndef SYNCWEBPAGE_H
#define SYNCWEBPAGE_H

#include <QObject>
#include <QAuthenticator>
#include <QtWebEngineWidgets/QWebEnginePage>

//------------------------------------------------------------------------------------//

using Authentication = std::pair<QString, QString>;

//------------------------------------------------------------------------------------//

class SyncWebPage : public QWebEnginePage
{
  
public:
  SyncWebPage();
  ~SyncWebPage();
  SyncWebPage(QObject *parent) : QWebEnginePage(parent) { }
  void updateConnInfo(QUrl url, Authentication authInfo);

private slots:
  void requireAuthentication(const QUrl & requestUrl, QAuthenticator * authenticator);

protected:
  virtual bool certificateError(const QWebEngineCertificateError & certificateError) override;
  
private:
  Authentication mAuthInfo;

};

#endif // SYNCWEBPAGE_H
//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
// EOF
//------------------------------------------------------------------------------------//
