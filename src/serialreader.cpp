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
#include "serialreader.h"

#include <QSerialPort>
#include <QXYSeries>

#include <QDebug>

SerialReader::SerialReader(QSerialPort *serialPort, QXYSeries *airSeries1,
                           QXYSeries *airSeries2, QXYSeries *airSeries3)
    : _serialPort(serialPort),
      _airSeries1(airSeries1),
      _airSeries2(airSeries2),
      _airSeries3(airSeries3)
{
    _airBuffer1.reserve(Samples);
    for (int i=0; i<Samples; ++i) {
        _airBuffer1.append(QPointF(i, 0));
        _airBuffer2.append(QPointF(i, 0));
        _airBuffer3.append(QPointF(i, 0));
    }
    _airSeries1->replace(_airBuffer1);
    _airSeries2->replace(_airBuffer2);
    _airSeries3->replace(_airBuffer3);
}

void SerialReader::read()
{
    auto data = _serialPort->readAll();
    for (auto line: data.split('\n')) {
        if (!(_position%Samples))
            _position=0;
        process(line.split(','));
        ++_position;
    }
    _airSeries1->replace(_airBuffer1);
    _airSeries2->replace(_airBuffer2);
    _airSeries3->replace(_airBuffer3);
    emit newData(data);
}

void SerialReader::process(const QList<QByteArray> &line)
{
    if (line.size() != 6)
        return;
    _airBuffer1[_position].setY(line[2].toInt());
    _airBuffer2[_position].setY(line[3].toInt());
    _airBuffer3[_position].setY(line[4].toInt());
}
