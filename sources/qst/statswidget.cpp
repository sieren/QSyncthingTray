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

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

namespace qst
{
namespace stats
{

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

const int StatsWidget::kMaxTrafficDataPoints = 60*60;
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
  mpDateTicker->setDateTimeFormat("mm:ss");
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
  plot->xAxis->setRangeReversed(true);
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
  disconnect(&mRedrawTimer, &QTimer::timeout, this,
    &StatsWidget::updatePlot);
  mRedrawTimer.stop();
}


//------------------------------------------------------------------------------------//

void StatsWidget::updateTrafficData(const TrafficData& traffData)
{
  std::lock_guard<std::mutex> lock(mTraffGuard);
  if (mTrafficPoints.size() > kMaxTrafficDataPoints)
  {
    mTrafficPoints.pop_front();
  }
  mTrafficPoints.push_back(traffData);
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

  const auto& currentTime = std::get<2>(mTrafficPoints.back());
  const auto& maxTime = duration_cast<seconds>(
    std::get<2>(mTrafficPoints.front()) - currentTime).count();
  auto idx = 0;
  for (auto& traffPoint : mTrafficPoints)
  {
    auto timeDelta = duration_cast<seconds>(currentTime - std::get<2>(traffPoint)).count();
    inTraff[idx] = std::get<0>(traffPoint);
    outTraff[idx] = std::get<1>(traffPoint);
    time[idx] = static_cast<double>(timeDelta);
    idx++;
  }

  mpCustomPlot->graph(0)->setData(time, outTraff);
  mpCustomPlot->graph(1)->setData(time, inTraff);

  const auto& maxOutTraffic = utilities::find_max_tuple_value<1>(mTrafficPoints);
  const auto& maxInTraffic = utilities::find_max_tuple_value<0>(mTrafficPoints);
  const auto maxTraffic = (std::max)(maxOutTraffic, maxInTraffic);

  mpCustomPlot->yAxis->setRange(0, maxTraffic);
  mpCustomPlot->xAxis->setRange(0, std::abs(maxTime));

  mpCustomPlot->replot();
}


//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

} // stats namespace
} // qst namespace

//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
