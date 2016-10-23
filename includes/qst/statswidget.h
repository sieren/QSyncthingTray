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

#ifndef STATSWIDGET_H
#define STATSWIDGET_H

#pragma once
#include <stdio.h>
#include <QObject>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QProcess>
#include <QWidget>
#include <QString>
#include <memory>
#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <utility>
#include "platforms.hpp"
#include "apihandler.hpp"
#include <contrib/qcustomplot.h>

namespace qst
{
namespace stats
{

class StatsWidget;
class StatsWidget : public QWidget
{
  Q_OBJECT;
public:
  StatsWidget() = delete;
  StatsWidget(const QString& title);
  void updateTrafficData(const TrafficData& traffData);
  void closeEvent(QCloseEvent * event);

public slots:
  void show();

private slots:
  void updatePlot();

private:
  void configurePlot(QCustomPlot* plot);
  QTimer mRedrawTimer;
  QLabel *mpLabel;
  QString mTitle;
  QWidget *mpWidget;
  std::mutex mTraffGuard;
  QCustomPlot *mpCustomPlot;
  QSharedPointer<QCPAxisTickerDateTime> mpDateTicker;
  std::list<TrafficData> mTrafficPoints;
  static const int kMaxTrafficDataPoints;
  static const QBrush kBackgroundColor;
  static const QColor kForegroundColor;
};

} // stats namespace
} // qst namespace

#endif
