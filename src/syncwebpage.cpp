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

#include <iostream>
#include "syncwebpage.h"

//------------------------------------------------------------------------------------//

SyncWebPage::SyncWebPage()
{
  connect(this, &QWebEnginePage::authenticationRequired,
    this, &SyncWebPage::requireAuthentication);
}


//------------------------------------------------------------------------------------//

void SyncWebPage::updateConnInfo(QUrl url, Authentication authInfo)
{
  mAuthInfo = authInfo;
  setUrl(url);
}


//------------------------------------------------------------------------------------//

void SyncWebPage::requireAuthentication(
  const QUrl &requestUrl, QAuthenticator *authenticator)
{
  authenticator->setUser(tr(mAuthInfo.first.c_str()));
  authenticator->setPassword(tr(mAuthInfo.first.c_str()));
}


//------------------------------------------------------------------------------------//

bool SyncWebPage::certificateError(const QWebEngineCertificateError &certificateError)
{
  return true; // TODO: Figure out whether there is a syncthing CA so we can use the
               // real certificate
}
