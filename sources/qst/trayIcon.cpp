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


#include <qst/appsettings.hpp>
#include <qst/identifiers.hpp>
#include <qst/trayIcon.h>
#include <QApplication>
#include <QDesktopWidget>

static const std::list<std::pair<std::string, std::string>> kIconSet(
  {{":/images/syncthingBlue.png", ":/images/syncthingGrey.png"},
   {":/images/syncthingBlack.png", ":/images/syncthingGrey.png"}});
static const std::list<std::string> kAnimatedIconSet(
  {":/images/syncthingBlueAnim.gif",
   ":/images/syncthingBlackAnim.gif"});

namespace qst
{
namespace ui
{
TrayIcon::TrayIcon(QObject *parent) :
  QSystemTrayIcon(parent)
, mpAppSettings(std::shared_ptr<settings::AppSettings>(new settings::AppSettings))
, mpTrayMenu(new TrayMenu(this, mpAppSettings))
, mpAnimatedIconMovie(new QMovie())
, mTrayMenuAction(mpTrayMenu->widget())
{
  setToolTip("Syncthing");
  setTrayIcon(0);
  mpMenu = new QMenu();
  mpMenu->addAction(&mTrayMenuAction);
  mTrayMenuAction.installEventFilter(this);
  connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}



void TrayIcon::setTrayIcon(int index)
{
  // temporary workaround as setIcon seems to leak memory
  // https://bugreports.qt.io/browse/QTBUG-23658?jql=text%20~%20%22setIcon%20memory%22
  // https://bugreports.qt.io/browse/QTBUG-16113?jql=text%20~%20%22setIcon%20memory%22
  
  QIcon icon;
  const bool iconMonochrome = mpAppSettings->value(kMonochromeIconId).toBool();
  std::pair<std::string, std::string> iconSet = iconMonochrome ?
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
    setIcon(icon);
  }
}


//------------------------------------------------------------------------------------//

void TrayIcon::iconActivated(const QSystemTrayIcon::ActivationReason reason)
{
  switch (reason)
  {
    case QSystemTrayIcon::Trigger:
      if(mpMenu->isVisible())
      {
        mpMenu->close();
      }
      else
      {
        mpMenu->popup(QCursor::pos());
        mpMenu->raise();
      }
      break;
    case QSystemTrayIcon::DoubleClick:
     //qst::sysutils::SystemUtility().doubleClicked(&Window::showWebView, this);
      break;
    case QSystemTrayIcon::MiddleClick:
      break;
    default:
      ;
  }
}


//------------------------------------------------------------------------------------//

bool TrayIcon::eventFilter(QObject *object, QEvent *event)
{
  if (event->type() == QEvent::FocusOut)
  {
    if (object == &mTrayMenuAction)
    {
      mpMenu->close();
    }
  }
  return false;
}


//------------------------------------------------------------------------------------//

void TrayIcon::onStartAnimation(const bool animate)
{
  const bool iconMonochrome = mpAppSettings->value(kMonochromeIconId).toBool();
  QString iconToAnimate = iconMonochrome ? tr(kAnimatedIconSet.back().c_str())
  : tr(kAnimatedIconSet.front().c_str());
  const bool animationEnabled = mpAppSettings->value(kIconAnimcationsEnabledId).toBool();
  if (!animate || !animationEnabled)
  {
    mShouldStopAnimation = true;
  }
  else if (animate && animationEnabled
           && mpAnimatedIconMovie->state() != QMovie::Running)
  {
    mShouldStopAnimation = false;
    mpAnimatedIconMovie->setFileName(iconToAnimate);
    mpAnimatedIconMovie->start();
  }
}


//------------------------------------------------------------------------------------//

void TrayIcon::onUpdateIcon()
{
  if (mShouldStopAnimation && mpAnimatedIconMovie->currentFrameNumber() ==
      mpAnimatedIconMovie->frameCount() - 1)
  {
    mpAnimatedIconMovie->stop();
  }
  setIcon(QIcon(QPixmap::fromImage(mpAnimatedIconMovie->currentImage())));
}
  
} // namespace ui
} // namespace qst
