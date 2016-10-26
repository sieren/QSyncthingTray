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

namespace
{
  using ConnectionPlotData = std::tuple<std::uint16_t, std::chrono::time_point<std::chrono::system_clock>>;
}
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
  void addConnectionPoint(const std::uint16_t& numConn);
  void closeEvent(QCloseEvent * event);
  void onSettingsChanged();

public slots:
  void show();

private slots:
  void updatePlot();

private:
  void configurePlot(QCustomPlot* plot, const QString& title);
  void updateTitle(QCustomPlot* plot, const QString& title);
  void updateTrafficPlot();
  void updateConnectionsPlot();

  template<typename Container, typename Duration>
  void cleanupTimeData(Container& vec, const Duration& dur);

  template<typename Container>
  void zeroMissingTimeData(Container& vec);

  QString mTitle;
  QSettings mSettings;
  QTimer mRedrawTimer;
  QLabel *mpLabel;
  QWidget *mpWidget;
  std::mutex mDataGuard;
  QCustomPlot *mpCustomPlot;
  QCustomPlot *mpConnectionPlot;
  QSharedPointer<QCPAxisTickerDateTime> mpDateTicker;
  std::list<TrafficData> mTrafficPoints;
  std::list<ConnectionPlotData> mConnectionPoints;
  int mMaxTimeInPlotMins = 60;
  static const int kMaxSecBeforeZero;
  static const QBrush kBackgroundColor;
  static const QColor kForegroundColor;
};

} // stats namespace
} // qst namespace

#endif
