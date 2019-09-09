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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serialreader.h"

#include <QMap>
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QChartGlobal>
#include <QTimer>

namespace Ui {
class MainWindow;
}
QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
QT_CHARTS_END_NAMESPACE

class QActionGroup;

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File
    void on_actionQuit_triggered();

    // Device
    void on_deviceMenu_aboutToShow();
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();

    // Help
    void on_actionAboutQt_triggered();

    // Other
    void showNewData(const QByteArray &data);

private:
    void setStandardBaudRates();
    void setSerialPortInfo();
    void setActionsForPortInfos();

private:
    Ui::MainWindow *_ui;
    QSerialPort *_serialPort;

    QMenu *_baudMenu;
    QMenu *_portMenu;

    QActionGroup *_baudGroup;
    QActionGroup *_portGroup;

    QMap<QString, QSerialPortInfo> _serialPortInfos;

    QTimer _timer;
    const int _timer_msec = 50;

    QLineSeries *_airSeries1;
    QLineSeries *_airSeries2;
    QLineSeries *_airSeries3;
    SerialReader _serialReader;
};

#endif // MAINWINDOW_H