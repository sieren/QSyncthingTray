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
  mpCustomPlot->setMinimumWidth(400);
  mpCustomPlot->setMinimumHeight(200);
  mpCustomPlot->plotLayout()->insertRow(0);
  mpCustomPlot->plotLayout()->addElement(0, 0,
    new QCPTextElement(mpCustomPlot, "Traffic (1hr)"));
  auto textElement = dynamic_cast<QCPTextElement*>(
     mpCustomPlot->plotLayout()->element(0, 0));
  textElement->setTextColor(kForegroundColor);

  pLayout->addWidget(mpCustomPlot);
  setLayout(pLayout);
  mpCustomPlot->addGraph();
  mpCustomPlot->addGraph();

  mpCustomPlot->graph(0)->setPen(QPen(QColor(255, 0, 0, 255)));
  mpCustomPlot->graph(0)->setName("Output");
  mpCustomPlot->graph(1)->setPen(QPen(QColor(0, 255, 255, 255)));
  mpCustomPlot->graph(1)->setName("Input");
  mpCustomPlot->xAxis->setLabel("Time");
  mpCustomPlot->yAxis->setLabel("kb/s");

  configurePlot(mpCustomPlot);
}


//------------------------------------------------------------------------------------//

void StatsWidget::configurePlot(QCustomPlot* plot)
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

void StatsWidget::updateTrafficData(const TrafficData& traffData)
{
  std::lock_guard<std::mutex> lock(mTraffGuard);
  mTrafficPoints.push_back(traffData);
  cleanupTimeData(mTrafficPoints, std::chrono::minutes{kMaxTimeInPlotMins});
  zeroMissingTimeData(mTrafficPoints);
}


//------------------------------------------------------------------------------------//

void StatsWidget::updatePlot()
{
  std::unique_lock<std::mutex> lock(mTraffGuard, std::try_to_lock);
  if(!lock.owns_lock())
  {
    return;
  }
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
