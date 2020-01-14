/*
 * Music Psychology Toolbox (MPT) - Chart
 *
 * Copyright (c) 2019 - 2020 Alexander Fust
 * Copyright (c) 2019 - 2020 Christopher Fust
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
#include "douglaspeucker.h"

#include <QtMath>

DouglasPeucker::DouglasPeucker()
{

}

void DouglasPeucker::douglasPeucker(const QVector<QPointF> &data, qreal epsilon,
                                    QVector<QPointF> &result)
{
    if (data.size()<2) {
        result = data;
        return;
    }

    qreal dmax = 0.0;
    int index = 0;

    for (int i=1; i<data.size()-1; ++i) {
        auto pd = perpendicularDistance(data[i], data.first(), data.last());
        if (pd > dmax) {
            index = i;
            dmax = pd;
        }
    }

    if (dmax > epsilon) {
        QVector<QPointF> result1;
        QVector<QPointF> result2;
        douglasPeucker(data.mid(0, index+1), epsilon,result1);
        douglasPeucker(data.mid(index), epsilon, result2);

        result = result1.mid(0, result1.length()-1);
        result.append(result2);
        return;
    }

    result.clear();
    result.push_back(data.first());
    result.push_back(data.last());
}

qreal DouglasPeucker::perpendicularDistance(const QPointF &point,
                                            const QPointF &start,
                                            const QPointF &end)
{
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();

    // normalize
    qreal mag = qSqrt(qPow(dx, 2.0) + qPow(dy, 2.0));
    if (mag > 0.0) {
        dx /= mag;
        dy /= mag;
    }

    qreal pvx = point.x() - start.x();
    qreal pvy = point.y() - start.y();

    // get dot product (project pv onto normalized direction)
    qreal pvdot = dx * pvx + dy * pvy;

    // scale line direction vector
    qreal dsx = pvdot * dx;
    qreal dsy = pvdot * dy;

    // substract this from pv
    qreal ax = pvx - dsx;
    qreal ay = pvy - dsy;

    return qSqrt(qPow(ax, 2.0) + qPow(ay, 2.0));
}
