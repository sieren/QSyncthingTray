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


#include "syncwebview.h"
#include <QContextMenuEvent>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <functional>

//------------------------------------------------------------------------------------//

SyncWebView::SyncWebView(QUrl url, Authentication authInfo) :
   mSyncThingUrl(url)
  ,mAuthInfo(authInfo)
  ,mContextMenu(this)
{
  initWebView();
  setupMenu();
}


//------------------------------------------------------------------------------------//

void SyncWebView::initWebView()
{
  QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
  profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
  mpPageView = new SyncWebPage(profile);

  QWebEngineSettings* settings = mpPageView->settings();
  settings->setAttribute(QWebEngineSettings::WebAttribute::JavascriptEnabled, true);
  settings->setAttribute(QWebEngineSettings::WebAttribute::AutoLoadImages,true);
  settings->setAttribute(QWebEngineSettings::WebAttribute::LocalContentCanAccessRemoteUrls, true);
  mpPageView->load(mSyncThingUrl);
  setPage(mpPageView);
}


//------------------------------------------------------------------------------------//

void SyncWebView::updateConnection(QUrl url, Authentication authInfo)
{
  mpPageView->updateConnInfo(url, authInfo);
  setPage(mpPageView);
  reload();
}


//------------------------------------------------------------------------------------//

void SyncWebView::closeEvent(QCloseEvent *event)
{
  QWebEngineView::closeEvent(event);
  mfk::sysutils::SystemUtility().showDockIcon(false);
}


//------------------------------------------------------------------------------------//

void SyncWebView::show()
{
  QWebEngineView::show();
  setFocusPolicy(Qt::ClickFocus);
  setEnabled(true);
  setFocus();
  mfk::sysutils::SystemUtility().showDockIcon(true);
}


//------------------------------------------------------------------------------------//

void SyncWebView::setupMenu()
{
  QAction *shrtCut = pageAction(QWebEnginePage::Cut);
  shrtCut->setShortcut(QKeySequence::Cut);
  QAction *shrtCopy = pageAction(QWebEnginePage::Copy);;
  shrtCopy->setShortcut(QKeySequence::Copy);
  QAction *shrtPaste = pageAction(QWebEnginePage::Paste);
  shrtPaste->setShortcut(QKeySequence::Paste);
  QAction *slctAll = pageAction(QWebEnginePage::SelectAll);
  slctAll->setShortcut(QKeySequence::SelectAll);
  
  using namespace std::placeholders;
  addActions(std::bind(&QWidget::addAction, &mContextMenu, _1),
             shrtCut, shrtCopy, shrtPaste, slctAll);
  addActions(std::bind(&QWebEngineView::addAction, this, _1),
             shrtCut, shrtCopy, shrtPaste, slctAll);
  
  mContextMenu.addSeparator();
  QAction *shrtReload = pageAction(QWebEnginePage::Reload);
  addActions(std::bind(&QWebEngineView::addAction, this, _1), shrtReload);
}


//------------------------------------------------------------------------------------//

void SyncWebView::contextMenuEvent(QContextMenuEvent * event)
{
  mContextMenu.exec(event->globalPos());
}


//------------------------------------------------------------------------------------//

template<typename F, typename T, typename... TArgs>
void SyncWebView::addActions(F &&funct, T action, TArgs... Actions)
{
  funct(action);
  addActions(funct, std::forward<TArgs>(Actions)...);
}


//------------------------------------------------------------------------------------//

template<typename F, typename T>
void SyncWebView::addActions(F &&funct, T action)
{
  funct(action);
}


//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
// EOF
//------------------------------------------------------------------------------------//
