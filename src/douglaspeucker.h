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
#ifndef DOUGLASPEUCKER_H
#define DOUGLASPEUCKER_H

#include <QVector>
#include <QPointF>

/**
 * @brief Ramer-Douglas-Peucker algorithm.
 *
 * @remark See <https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm>
 *         and <https://rosettacode.org/wiki/Ramer-Douglas-Peucker_line_simplification>
 */
class DouglasPeucker final
{
private:
    DouglasPeucker();

public:
    static void douglasPeucker(const QVector<QPointF> &data, qreal epsilon,
                               QVector<QPointF> &result);

private:
    static qreal perpendicularDistance(const QPointF &point,
                                       const QPointF &start,
                                       const QPointF &end);
};

#endif // DOUGLASPEUCKER_H
