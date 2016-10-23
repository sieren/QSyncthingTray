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

#include <qst/statswidget.h>
#include <qst/utilities.hpp>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include <algorithm>
#include <cassert>

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

namespace qst
{
namespace stats
{

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

const int StatsWidget::kMaxTimeInPlotMins = 60;
const int StatsWidget::kMaxSecBeforeZero = 10;
const QBrush StatsWidget::kBackgroundColor{QColor(0,0,0,255)};
const QColor StatsWidget::kForegroundColor{255,255,255,255};

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

StatsWidget::StatsWidget(const QString& title) :
    mTitle(title)
  , mpDateTicker(new QCPAxisTickerDateTime)
{
  mpWidget = new QWidget();
  QVBoxLayout* pLayout = new QVBoxLayout();
  setStyleSheet("background-color:black;");
  mpDateTicker->setDateTimeFormat("hh:mm");
  mpLabel = new QLabel (title);

  mpCustomPlot = new QCustomPlot();
  mpConnectionPlot = new QCustomPlot();

  // Traffic Plot
  mpCustomPlot->addGraph();
  mpCustomPlot->addGraph();

  mpCustomPlot->graph(0)->setPen(QPen(QColor(255, 0, 0, 255)));
  mpCustomPlot->graph(0)->setName("Output");
  mpCustomPlot->graph(1)->setPen(QPen(QColor(0, 255, 255, 255)));
  mpCustomPlot->graph(1)->setName("Input");
  mpCustomPlot->xAxis->setLabel("Time");
  mpCustomPlot->yAxis->setLabel("kb/s");

  // Traffic Plot
  mpConnectionPlot->addGraph();

  mpConnectionPlot->graph(0)->setPen(QPen(QColor(255, 0, 0, 255)));
  mpConnectionPlot->graph(0)->setName("Active Connections");
  mpConnectionPlot->xAxis->setLabel("Time");
  mpConnectionPlot->yAxis->setLabel("Connections");

  configurePlot(mpCustomPlot, "Traffic (1hr)");
  configurePlot(mpConnectionPlot, "Connections (1hr)");


  pLayout->addWidget(mpCustomPlot);
  pLayout->addWidget(mpConnectionPlot);
  setLayout(pLayout);
}


//------------------------------------------------------------------------------------//

void StatsWidget::configurePlot(QCustomPlot* plot, const QString& title)
{
  plot->yAxis->setNumberFormat("f");
  plot->yAxis->setNumberPrecision(1);
  plot->xAxis->setTicker(mpDateTicker);
  plot->xAxis->setLabelColor(kForegroundColor);
  plot->yAxis->setLabelColor(kForegroundColor);
  plot->xAxis->setTickLabelColor(kForegroundColor);
  plot->yAxis->setTickLabelColor(kForegroundColor);
  plot->setBackground(kBackgroundColor);
  plot->legend->setFont(QFont(font().family(), 7));
  plot->legend->setIconSize(15, 10);
  plot->legend->setVisible(true);
  plot->legend->setTextColor(kForegroundColor);
  plot->legend->setBrush(kBackgroundColor);
  plot->legend->setBorderPen(QPen(kForegroundColor));
  plot->xAxis->grid()->setVisible(true);
  plot->xAxis->grid()->setPen(QPen(kForegroundColor, 0, Qt::DotLine));
  plot->yAxis->grid()->setVisible(true);
  plot->yAxis->grid()->setPen(QPen(kForegroundColor, 0, Qt::DotLine));
  plot->xAxis->setBasePen(QPen(kForegroundColor, 0, Qt::SolidLine));
  plot->yAxis->setBasePen(QPen(kForegroundColor, 0, Qt::SolidLine));

  plot->setMinimumWidth(400);
  plot->setMinimumHeight(200);
  plot->plotLayout()->insertRow(0);
  plot->plotLayout()->addElement(0, 0,
    new QCPTextElement(plot, title));
  auto textElement = dynamic_cast<QCPTextElement*>(
    plot->plotLayout()->element(0, 0));
  textElement->setTextColor(kForegroundColor);

}


//------------------------------------------------------------------------------------//

void StatsWidget::show()
{
  if (isVisible())
  {
    QWidget::show();
    QWidget::activateWindow();
    QWidget::raise();
    return;
  }
  QWidget::show();
  connect(&mRedrawTimer, &QTimer::timeout, this,
    &StatsWidget::updatePlot);
  mRedrawTimer.start(1000);
}


//------------------------------------------------------------------------------------//

void StatsWidget::closeEvent(QCloseEvent* event)
{
  QWidget::closeEvent(event);
  disconnect(&mRedrawTimer, &QTimer::timeout, this,
    &StatsWidget::updatePlot);
  mRedrawTimer.stop();
}


//------------------------------------------------------------------------------------//

void StatsWidget::addConnectionPoint(const std::uint16_t& numConn)
{
  using namespace std::chrono;
  std::lock_guard<std::mutex> lock(mDataGuard);
  mConnectionPoints.emplace_back(std::make_tuple(numConn, system_clock::now()));
  cleanupTimeData(mConnectionPoints, std::chrono::minutes{kMaxTimeInPlotMins});
  zeroMissingTimeData(mConnectionPoints);
}


//------------------------------------------------------------------------------------//

void StatsWidget::updateTrafficData(const TrafficData& traffData)
{
  std::lock_guard<std::mutex> lock(mDataGuard);
  mTrafficPoints.push_back(traffData);
  cleanupTimeData(mTrafficPoints, std::chrono::minutes{kMaxTimeInPlotMins});
  zeroMissingTimeData(mTrafficPoints);
}


//------------------------------------------------------------------------------------//

void StatsWidget::updatePlot()
{
  std::unique_lock<std::mutex> lock(mDataGuard, std::try_to_lock);
  if(!lock.owns_lock())
  {
    return;
  }
  updateTrafficPlot();
  updateConnectionsPlot();
}


//------------------------------------------------------------------------------------//

void StatsWidget::updateConnectionsPlot()
{
  const auto numPoints = static_cast<double>(mConnectionPoints.size());
  QVector<double> connections(numPoints), time(numPoints);

  using namespace std::chrono;

  const auto& maxTime = duration_cast<seconds>(std::get<1>(
    mConnectionPoints.back()).time_since_epoch()).count();
  const auto& minTime = duration_cast<seconds>(std::get<1>(
    mConnectionPoints.front()).time_since_epoch()).count();

  auto idx = 0;
  for (auto& connPoint : mConnectionPoints)
  {
    const auto& timePoint =
      duration_cast<seconds>(std::get<1>(connPoint).time_since_epoch()).count();
    connections[idx] = std::get<0>(connPoint);
    time[idx] = static_cast<double>(timePoint);
    idx++;
  }

  mpConnectionPlot->graph(0)->setData(time, connections);

  auto cmp = [](const ConnectionPlotData& lhs, const ConnectionPlotData& rhs)
  {
    return std::get<0>(lhs) < std::get<0>(rhs);
  };
  const auto& maxConns = std::max_element(
    mConnectionPoints.begin(), mConnectionPoints.end(), cmp);
  mpConnectionPlot->yAxis->setRange(0, std::get<0>(*maxConns));
  mpConnectionPlot->xAxis->setRange(minTime, maxTime);

  mpConnectionPlot->replot();
}


//------------------------------------------------------------------------------------//

void StatsWidget::updateTrafficPlot()
{
  const auto numPoints = static_cast<double>(mTrafficPoints.size());
  QVector<double> inTraff(numPoints);
  QVector<double> outTraff(numPoints), time(numPoints);

  using namespace std::chrono;

  const auto& maxTime = duration_cast<seconds>(std::get<2>(
    mTrafficPoints.back()).time_since_epoch()).count();
  const auto& minTime = duration_cast<seconds>(std::get<2>(
    mTrafficPoints.front()).time_since_epoch()).count();

  auto idx = 0;
  for (auto& traffPoint : mTrafficPoints)
  {
    const auto& timePoint =
      duration_cast<seconds>(std::get<2>(traffPoint).time_since_epoch()).count();
    inTraff[idx] = std::get<0>(traffPoint);
    outTraff[idx] = std::get<1>(traffPoint);
    time[idx] = static_cast<double>(timePoint);
    idx++;
  }

  mpCustomPlot->graph(0)->setData(time, outTraff);
  mpCustomPlot->graph(1)->setData(time, inTraff);

  const auto& maxOutTraffic = utilities::find_max_tuple_value<1>(mTrafficPoints);
  const auto& maxInTraffic = utilities::find_max_tuple_value<0>(mTrafficPoints);
  const auto maxTraffic = (std::max)(maxOutTraffic, maxInTraffic);

  mpCustomPlot->yAxis->setRange(0, maxTraffic);
  mpCustomPlot->xAxis->setRange(minTime, maxTime);

  mpCustomPlot->replot();
}


//------------------------------------------------------------------------------------//

template<typename Container, typename Duration>
void StatsWidget::cleanupTimeData(Container& vec, const Duration& dur)
{
  using namespace std::chrono;
  using TimeValueType = std::chrono::time_point<std::chrono::system_clock>;
  using ContainerValueType = typename Container::value_type;
  const auto TimeValueTypePosition =
    utilities::Index<TimeValueType, ContainerValueType>::value;

  const auto& maxTime = std::get<TimeValueTypePosition>(vec.back());
  const auto minTime = maxTime - dur;
  const auto timeCmp = [](const ContainerValueType& obj, const TimeValueType& time)
    {
      // MSVC2013 doesnt accept the capture
      const auto pos = utilities::Index<TimeValueType, ContainerValueType>::value;
      return std::get<pos>(obj) < time;
    };
  const auto endExpired = std::lower_bound(
    std::begin(vec), std::end(vec), minTime, timeCmp);
  vec.erase(std::begin(vec), endExpired);
}


//------------------------------------------------------------------------------------//

template<typename Container>
void StatsWidget::zeroMissingTimeData(Container &vec)
{
  using namespace std::chrono;
  using TimeValueType = std::chrono::time_point<std::chrono::system_clock>;
  using ContainerValueType = typename Container::value_type;
  const auto TimeValueTypePosition =
    utilities::Index<TimeValueType, ContainerValueType>::value;

  const auto maxTimeDistance = seconds(kMaxSecBeforeZero);
  auto timeCmp = [&maxTimeDistance]
    (const ContainerValueType& lhs, const ContainerValueType& rhs)
    {
      const auto pos =
        utilities::Index<TimeValueType, ContainerValueType>::value;
      return duration_cast<seconds>(
        std::get<pos>(rhs) - std::get<pos>(lhs)) > maxTimeDistance;
    };

  const auto itDist = std::adjacent_find(vec.begin(), vec.end(), timeCmp);
  const auto itDistEnd = std::next(itDist, 1);
  if (itDist == vec.end())
  {
    return;
  }
  assert(itDistEnd != vec.end());
  const auto& timePointStart = std::get<TimeValueTypePosition>(*itDist);
  const auto& timePointEnd = std::get<TimeValueTypePosition>(*itDistEnd);
  const auto newTimeStart = timePointStart + seconds(kMaxSecBeforeZero/2);
  const auto newTimeEnd = timePointEnd - seconds(kMaxSecBeforeZero/2);

  auto newStartElement = ContainerValueType();
  auto newEndElement = ContainerValueType();
  std::get<TimeValueTypePosition>(newStartElement) = newTimeStart;
  std::get<TimeValueTypePosition>(newEndElement) = newTimeEnd;
  const std::vector<ContainerValueType> newVec{newStartElement, newEndElement};
  vec.insert(itDistEnd, newVec.begin(), newVec.end());
  zeroMissingTimeData(vec);
}

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

} // stats namespace
} // qst namespace

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
