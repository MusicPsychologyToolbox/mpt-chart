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
#include "douglaspeucker.h"

#include <QSerialPort>
#include <QLineSeries>
#include <QValueAxis>
#include <QXYSeries>

SerialReader::SerialReader(QObject *parent)
    : QObject(parent)
    , _serialPort(new QSerialPort(this))
    , _airSeries1(new QLineSeries(this))
    , _airSeries2(new QLineSeries(this))
    , _airSeries3(new QLineSeries(this))
    , _pulseSeries(new QLineSeries(this))
{
    _airSeries1->setName("air1");
    _airSeries2->setName("air2");
    _airSeries3->setName("air3");
    _pulseSeries->setName("pulse");

    _airBuffer1.reserve(_samples);
    _airBuffer2.reserve(_samples);
    _airBuffer3.reserve(_samples);
    _pulseBuffer.reserve(_samples);
}

SerialReader::~SerialReader()
{
    if (_serialPort->isOpen())
        _serialPort->close();
    delete _serialPort;
}

void SerialReader::clear()
{
    _arduinoReady = false;
    _airBuffer1.clear();
    _airBuffer2.clear();
    _airBuffer3.clear();
    _pulseBuffer.clear();
}

void SerialReader::showPulse(bool show)
{
    _showPulse = show;
}

void SerialReader::load(const QList<QByteArray> &lines)
{
    clear();
    process(lines);
}

void SerialReader::reload()
{
    process({});
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
        if (_arduinoReady) {
            lines.push_back(line);
        } else {
            _arduinoReady = line.contains("Arduino Ready");
            if (_arduinoReady)
                emit arduinoStarted();
        }
    }

    for (auto line: lines) {
        appendValues(line.split(','));
    }

    QVector<QPointF> dprAir1;
    DouglasPeucker::douglasPeucker(_airBuffer1, _dgEpsilon, dprAir1);
    _airSeries1->replace(dprAir1);
    QVector<QPointF> dprAir2;
    DouglasPeucker::douglasPeucker(_airBuffer2, _dgEpsilon, dprAir2);
    _airSeries2->replace(dprAir2);
    QVector<QPointF> dprAir3;
    DouglasPeucker::douglasPeucker(_airBuffer3, _dgEpsilon, dprAir3);
    _airSeries3->replace(dprAir3);
    QVector<QPointF> dprPulse;
    DouglasPeucker::douglasPeucker(_pulseBuffer, _dgEpsilon, dprPulse);
    _pulseSeries->replace(dprPulse);

    if (!dprAir1.isEmpty())
        _axisX->setMax(dprAir1.last().x());

    if (!lines.isEmpty())
        emit newData(lines.join("\n"));
}

void SerialReader::appendValues(const QList<QByteArray> &columns)
{
    if (columns.size() != 6)
        return;
    auto ms = columns[0].toDouble();
    _airBuffer1.append(QPointF(ms, columns[2].toInt()));
    _airBuffer2.append(QPointF(ms, columns[3].toInt()));
    _airBuffer3.append(QPointF(ms, columns[4].toInt()));
    if (_showPulse && columns.size() == 6)
        _pulseBuffer.append(QPointF(ms, columns[5].toInt()));
}

void SerialReader::process(const QList<QByteArray> &lines)
{
    for (auto line: lines)
        appendValues(line.split(','));

    QVector<QPointF> dprAir1;
    DouglasPeucker::douglasPeucker(_airBuffer1, _dgEpsilon, dprAir1);
    _airSeries1->replace(dprAir1);
    QVector<QPointF> dprAir2;
    DouglasPeucker::douglasPeucker(_airBuffer2, _dgEpsilon, dprAir2);
    _airSeries2->replace(dprAir2);
    QVector<QPointF> dprAir3;
    DouglasPeucker::douglasPeucker(_airBuffer3, _dgEpsilon, dprAir3);
    _airSeries3->replace(dprAir3);
    QVector<QPointF> dprPulse;
    DouglasPeucker::douglasPeucker(_pulseBuffer, _dgEpsilon, dprPulse);
    _pulseSeries->replace(dprPulse);

    if (!dprAir1.isEmpty())
        _axisX->setMax(dprAir1.last().x());
}
