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
#include <QLineSeries>
#include <QXYSeries>

SerialReader::SerialReader(QSerialPort *serialPort, QObject *parent)
    : QObject(parent),
      _serialPort(serialPort),
      _airSeries1(new QLineSeries(this)),
      _airSeries2(new QLineSeries(this)),
      _airSeries3(new QLineSeries(this)),
      _pulseSeries(new QLineSeries(this))
{
    _airSeries1->setName("air1");
    _airSeries2->setName("air2");
    _airSeries3->setName("air3");
    _pulseSeries->setName("pulse");

    _airBuffer1.reserve(_samples);
    _airBuffer2.reserve(_samples);
    _airBuffer3.reserve(_samples);
    _pulseBuffer.reserve(_samples);
    for (int i=0; i<_samples; ++i) {
        _airBuffer1.append(QPointF(i, 0));
        _airBuffer2.append(QPointF(i, 0));
        _airBuffer3.append(QPointF(i, 0));
        _pulseBuffer.append(QPointF(i, 0));
    }
    _airSeries1->replace(_airBuffer1);
    _airSeries2->replace(_airBuffer2);
    _airSeries3->replace(_airBuffer3);
    _pulseSeries->replace(_pulseBuffer);
}

void SerialReader::showPulse(bool show)
{
    _showPulse = show;
    for (auto i=0; i<_pulseBuffer.size(); ++i)
        _pulseBuffer[i].setY(0);
}

void SerialReader::process(const QList<QByteArray> &lines)
{
    _position = 0;
    for (auto line: lines) {
        if (!(_position % _samples)) {
            _position = 0;
        }
        setValues(line.split(','));
        ++_position;
    }

    _airSeries1->replace(_airBuffer1);
    _airSeries2->replace(_airBuffer2);
    _airSeries3->replace(_airBuffer3);
    _pulseSeries->replace(_pulseBuffer);
}

void SerialReader::read()
{
    _buffer.append(_serialPort->readAll());
    int index = _buffer.lastIndexOf('\n') + 1;
    auto data = _buffer.left(index);
    _buffer.remove(0, index);

    QList<QByteArray> lines;
    for (auto line: data.split('\n')) {
        if (line.isEmpty())
            continue;
        lines.push_back(line);
    }

    int count = lines.count();
    if (_samples - (_position + 1) < count) {
        _position = _samples - count;
        for (int i=0; i<_position; ++i) {
            _airBuffer1[i].setY(_airBuffer1.at(i+count).y());
            _airBuffer2[i].setY(_airBuffer2.at(i+count).y());
            _airBuffer3[i].setY(_airBuffer3.at(i+count).y());
            _pulseBuffer[i].setY(_pulseBuffer.at(i+count).y());
        }
    }

    for (auto line: lines) {
        if (setValues(line.split(',')))
            ++_position;
    }

    _airSeries1->replace(_airBuffer1);
    _airSeries2->replace(_airBuffer2);
    _airSeries3->replace(_airBuffer3);
    _pulseSeries->replace(_pulseBuffer);
    emit newData(lines.join("\n"));
}

bool SerialReader::setValues(const QList<QByteArray> &columns)
{
    if (columns.size() != 6)
        return false;
    _airBuffer1[_position].setY(columns[2].toInt());
    _airBuffer2[_position].setY(columns[3].toInt());
    _airBuffer3[_position].setY(columns[4].toInt());
    if (_showPulse && columns.size() == 6)
        _pulseBuffer[_position].setY(columns[5].toInt());
    return true;
}
