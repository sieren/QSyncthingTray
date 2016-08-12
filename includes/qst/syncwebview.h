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
#include <QSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <memory>
#include "utilities.hpp"
#include "syncwebpage.h"

//------------------------------------------------------------------------------------//

namespace qst
{
namespace webview
{

//------------------------------------------------------------------------------------//

static const bool kWebViewSupportsZoom = true;

//------------------------------------------------------------------------------------//

class SyncWebView : public QWebEngineView
{
  Q_OBJECT;
public:
  SyncWebView() = default;
  SyncWebView(QUrl url, Authentication authInfo);
  ~SyncWebView() = default;

  void show();
  void updateConnection(QUrl url, Authentication authInfo);

signals:
  void close();

private:
  void initWebView();
  void setupMenu();
  void pageHasLoaded(bool hasLoaded);
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  virtual void contextMenuEvent(QContextMenuEvent * event) override;

  std::unique_ptr<SyncWebPage> mpPageView;
  QUrl mSyncThingUrl;
  Authentication mAuthInfo;
  
  QMenu mContextMenu;
  QSettings mSettings;
  
  std::unique_ptr<QAction> shrtCut;
  std::unique_ptr<QAction> shrtPaste;
  std::unique_ptr<QAction> shrtCopy;
  std::unique_ptr<QAction> slctAll;


  template<typename F, typename T, typename... TArgs>
  void addActions(F &&funct, T action, TArgs... Actions);
  
  template<typename F, typename T>
  void addActions(F &&funct, T action);
};


} // qst namespace
} // webview namespace

#endif // SYNCWEBVIEW_H

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
// EOF
//------------------------------------------------------------------------------------//
