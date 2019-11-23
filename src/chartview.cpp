/*
 * Music Psychology Toolbox (MPT) - Chart
 *
 * Copyright (c) 2019 Alexander Fust
 * Copyright (c) 2019 Christopher Fust
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * This program is free software: you can redistribute it and/or modify
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "chartview.h"

#include <QValueAxis>

ChartView::ChartView(QWidget *parent)
    : QChartView(parent),
      _axisX(new QValueAxis),
      _axisY(new QValueAxis)
{
    chart()->addAxis(_axisX, Qt::AlignBottom);
    chart()->addAxis(_axisY, Qt::AlignLeft);
}

QValueAxis* ChartView::axisX() const
{
    return _axisX;
}

QValueAxis* ChartView::axisY() const
{
    return _axisY;
}

bool ChartView::viewportEvent(QEvent *event)
{
    return QChartView::viewportEvent(event);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        emit shiftLeft();
        break;
    case Qt::Key_Right:
        emit shiftRight();
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
    event->accept();
}
