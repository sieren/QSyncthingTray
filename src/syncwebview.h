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

#ifndef SYNCWEBVIEW_H
#define SYNCWEBVIEW_H

#include <QDialog>
#include <QMenu>
#include <QObject>
#include <QtWebEngineWidgets/QWebEngineView>
#include "utilities.hpp"
#include "syncwebpage.h"

//------------------------------------------------------------------------------------//

class SyncWebView : public QWebEngineView
{

public:
  SyncWebView() = default;
  SyncWebView(QUrl url, Authentication authInfo);
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  void show();
  virtual void contextMenuEvent(QContextMenuEvent * event) override;
  void updateConnection(QUrl url, Authentication authInfo);

private:
  void initWebView();
  void setupMenu();
  
  SyncWebPage *mpPageView;
  QUrl mSyncThingUrl;
  Authentication mAuthInfo;
  
  QMenu mContextMenu;
  
  template<typename F, typename T, typename... TArgs>
  void addActions(F &&funct, T action, TArgs... Actions);
  
  template<typename F, typename T>
  void addActions(F &&funct, T action);
};

#endif // SYNCWEBVIEW_H
