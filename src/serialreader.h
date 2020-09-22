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
#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QObject>
#include <QChartGlobal>
#include <QPointF>
#include <QVector>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;
class QXYSeries;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class QSerialPort;

class SerialReader : public QObject
{
    Q_OBJECT

public:
    SerialReader(QObject *parent = nullptr);
    ~SerialReader();

    void clear();

    void setAxisX(QValueAxis *axisX) {
        _axisX = axisX;
    }

    qreal dpEpsilon() const {
        return _dgEpsilon;
    }

    void setDpEpsilon(qreal epsilon) {
        _dgEpsilon = epsilon;
    }

    QSerialPort* serialPort() const {
        return _serialPort;
    }

    QXYSeries* airSeries1() const {
        return _airSeries1;
    }

    QXYSeries* airSeries2() const {
        return _airSeries2;
    }

    QXYSeries* airSeries3() const {
        return _airSeries3;
    }

    QXYSeries* pulseSeries() const {
        return _pulseSeries;
    }

    int samples() const {
        return _samples;
    }

    void setSamples(int samples) {
        _samples = samples;
    }

    void showPulse(bool show);

    void load(const QList<QByteArray> &lines);
    void reload();

signals:
    void newData(const QByteArray &data);
    void arduinoStarted();

public slots:
    void read();

private:
    void appendValues(const QList<QByteArray> &columns);
    void process(const QList<QByteArray> &lines);

private:
    bool _arduinoReady = false;
    bool _showPulse = false;
    int _position = 0;
    int _samples = 1000;
    qreal _dgEpsilon = 2.0;

    QSerialPort *_serialPort;

    QByteArray _buffer;

    QXYSeries *_airSeries1;
    QXYSeries *_airSeries2;
    QXYSeries *_airSeries3;
    QXYSeries *_pulseSeries;
    QVector<QPointF> _airBuffer1;
    QVector<QPointF> _airBuffer2;
    QVector<QPointF> _airBuffer3;
    QVector<QPointF> _pulseBuffer;
    QValueAxis *_axisX;
};

#endif // SERIALREADER_H
