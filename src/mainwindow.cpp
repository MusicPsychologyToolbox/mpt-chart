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
#include <QFileDialog>
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

    _minSpinBox = new QSpinBox(_ui->toolBar);
    _minSpinBox->setRange(0, 1024);
    _minSpinBox->setValue(200);
    auto minLabel = new QLabel("Minimum Y: ", _ui->toolBar);
    minLabel->setMargin(6);
    _ui->toolBar->addWidget(minLabel);
    _ui->toolBar->addWidget(_minSpinBox);
    _maxSpinBox = new QSpinBox(_ui->toolBar);
    _maxSpinBox->setRange(0, 1024);
    _maxSpinBox->setValue(300);
    auto maxLabel = new QLabel("Maximum Y: ", _ui->toolBar);
    maxLabel->setMargin(6);
    _ui->toolBar->addWidget(maxLabel);
    _ui->toolBar->addWidget(_maxSpinBox);
    connect(_minSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::minYChanged);
    connect(_maxSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::maxYChanged);

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
    _axisY = new QValueAxis;
    _axisY->setRange(_minSpinBox->value(), _maxSpinBox->value());
    _ui->chartView->chart()->addAxis(axisX, Qt::AlignBottom);
    _ui->chartView->chart()->addAxis(_axisY, Qt::AlignLeft);

    _airSeries1->attachAxis(axisX);
    _airSeries1->attachAxis(_axisY);
    _airSeries2->attachAxis(axisX);
    _airSeries2->attachAxis(_axisY);
    _airSeries3->attachAxis(axisX);
    _airSeries3->attachAxis(_axisY);
    _pulseSeries->attachAxis(axisX);
    _pulseSeries->attachAxis(_axisY);

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

void MainWindow::on_actionExportCSV_triggered()
{
    auto fileName = QFileDialog::getSaveFileName(this,
                                                 tr("Export CSV"),
                                                 QString(),
                                                 tr("CSV (*.csv)"));
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << "ms,sync,air1,air2,air3,pulse\n";
        stream << _rawData;
        file.close();
    }
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
    _rawData.clear();
    _rawData.reserve(_initSize);
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
    _rawData.append(data);
    _ui->dataLog->appendPlainText(data);
}

void MainWindow::minYChanged(int value)
{
    if (value == _maxSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be != maximum Y."));
    } else if (value > _maxSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be < maximum Y."));
    } else {
        _axisY->setMin(value);
    }
}

void MainWindow::maxYChanged(int value)
{
    if (value == _minSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be != maximum Y."));
    } else if (value < _minSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be < maximum Y."));
    } else {
        _axisY->setMax(value);
    }
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
