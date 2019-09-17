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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QMessageBox>
#include <QSerialPort>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _serialPort(new QSerialPort(this)),
    _airSeries1(new QLineSeries(this)),
    _airSeries2(new QLineSeries(this)),
    _airSeries3(new QLineSeries(this)),
    _pulseSeries(new QLineSeries(this)),
    _serialReader(_serialPort, _airSeries1, _airSeries2, _airSeries3, _pulseSeries)
{
    _ui->setupUi(this);
    setStandardBaudRates();
    setSerialPortInfo();

    _airSeries1->setName("air1");
    _airSeries2->setName("air2");
    _airSeries3->setName("air3");
    _pulseSeries->setName("pulse");
    _ui->chartView->chart()->addSeries(_airSeries1);
    _ui->chartView->chart()->addSeries(_airSeries2);
    _ui->chartView->chart()->addSeries(_airSeries3);
    _ui->chartView->chart()->addSeries(_pulseSeries);

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, _serialReader.Samples);
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0, 1023);
    _ui->chartView->chart()->addAxis(axisX, Qt::AlignBottom);
    _ui->chartView->chart()->addAxis(axisY, Qt::AlignLeft);

    _airSeries1->attachAxis(axisX);
    _airSeries1->attachAxis(axisY);
    _airSeries2->attachAxis(axisX);
    _airSeries2->attachAxis(axisY);
    _airSeries3->attachAxis(axisX);
    _airSeries3->attachAxis(axisY);
    _pulseSeries->attachAxis(axisX);
    _pulseSeries->attachAxis(axisY);

    connect(&_timer, &QTimer::timeout, &_serialReader, &SerialReader::read);
    connect(&_serialReader, &SerialReader::newData, this, &MainWindow::showNewData);
}

MainWindow::~MainWindow()
{
    if (_serialPort->isOpen())
        _serialPort->close();

    delete _serialPort;
    delete _ui;
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_deviceMenu_aboutToShow()
{
    _portMenu->clear();
    _serialPortInfos.clear();
    setActionsForPortInfos();
}

void MainWindow::on_actionConnect_triggered()
{
    if (_serialPort->isOpen())
        return;
    _ui->dataLog->clear();

    auto baud = _baudGroup->checkedAction()->text().toInt();
    if (!_serialPort->setBaudRate(baud, QSerialPort::AllDirections)) {
        _ui->statusLog->appendPlainText(QString("Error code: %1\n Message: %2")
                                        .arg(_serialPort->error())
                                        .arg(_serialPort->errorString()));
    }

    auto action = _portGroup->checkedAction();
    if (!action) return;
    auto portInfo = _serialPortInfos.value(action->text());
    _ui->statusLog->appendPlainText(QString("Configured Baud (%1) and Port (%2).")
                                    .arg(baud).arg(portInfo.portName()));
    if (portInfo.isNull()) {
        _ui->statusLog->appendPlainText("ERROR: no valid serial port found!");
    } else {
        _serialPort->setPort(portInfo);
    }

    _ui->statusLog->appendPlainText(QString("Start reading data. Baud: %1 Port: %2")
                                    .arg(_serialPort->baudRate())
                                    .arg(_serialPort->portName()));
    if (!_serialPort->open(QIODevice::ReadOnly)) {
        _ui->statusLog->appendPlainText(QString("Failed to open port %1 error: %2")
                                        .arg(_serialPort->portName())
                                        .arg(_serialPort->errorString()));
    }
    _timer.start(_timer_msec);
}

void MainWindow::on_actionDisconnect_triggered()
{
    if (!_serialPort->isOpen())
        return;
    _serialPort->close();
    _timer.stop();
    _ui->statusLog->appendPlainText("Data recoding stopped.");
}

void MainWindow::on_actionPulse_triggered()
{
    _serialReader.showPulse(_ui->actionPulse->isChecked());
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::showNewData(const QByteArray &data)
{
    _ui->dataLog->appendPlainText(data);
}

void MainWindow::setStandardBaudRates()
{
    _baudMenu = new QMenu("Baud", this);
    _baudGroup = new QActionGroup(_baudMenu);
    _baudGroup->setExclusive(true);
    auto baudRates = QSerialPortInfo::standardBaudRates();
    for (auto baudRate: baudRates) {
        auto baud = new QAction(QString::number(baudRate), _baudMenu);
        baud->setCheckable(true);
        if (baudRate == 115200) baud->setChecked(true);
        _baudMenu->addAction(baud);
        _baudGroup->addAction(baud);
    }
    _ui->deviceMenu->addMenu(_baudMenu);
}

void MainWindow::setSerialPortInfo()
{
    _portMenu = new QMenu("Port", this);
    _portGroup = new QActionGroup(_portMenu);
    _portGroup->setExclusive(true);
    setActionsForPortInfos();
    _ui->deviceMenu->addMenu(_portMenu);
}

void MainWindow::setActionsForPortInfos()
{
    auto portInfos = QSerialPortInfo::availablePorts();
    QSerialPortInfo portInfo;
    for (int i=0; i<portInfos.size(); ++i) {
        portInfo = portInfos.at(i);
        auto port = new QAction(portInfo.portName(), _portMenu);
        port->setCheckable(true);
        if (i == 0) port->setChecked(true);
        _serialPortInfos[portInfo.portName()] = portInfo;
        _portMenu->addAction(port);
        _portGroup->addAction(port);
    }
}
