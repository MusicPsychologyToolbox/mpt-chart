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
#include <QLabel>
#include <QMessageBox>
#include <QSerialPort>
#include <QSpinBox>
#include <QTextStream>
#include <QValueAxis>
#include <QXYSeries>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _serialPort(new QSerialPort(this)),
    _serialReader(_serialPort)
{
    _ui->setupUi(this);
    setStandardBaudRates();
    setSerialPortInfo();

    _ui->chartView->chart()->addSeries(_serialReader.airSeries1());
    _ui->chartView->chart()->addSeries(_serialReader.airSeries2());
    _ui->chartView->chart()->addSeries(_serialReader.airSeries3());
    _ui->chartView->chart()->addSeries(_serialReader.pulseSeries());

    setupAxisX();
    setupAxisY();

    _serialReader.airSeries1()->attachAxis(_ui->chartView->axisX());
    _serialReader.airSeries1()->attachAxis(_ui->chartView->axisY());
    _serialReader.airSeries2()->attachAxis(_ui->chartView->axisX());
    _serialReader.airSeries2()->attachAxis(_ui->chartView->axisY());
    _serialReader.airSeries3()->attachAxis(_ui->chartView->axisX());
    _serialReader.airSeries3()->attachAxis(_ui->chartView->axisY());
    _serialReader.pulseSeries()->attachAxis(_ui->chartView->axisX());
    _serialReader.pulseSeries()->attachAxis(_ui->chartView->axisY());

    connect(&_timer, &QTimer::timeout, &_serialReader, &SerialReader::read);
    connect(&_serialReader, &SerialReader::newData,
            this, &MainWindow::showNewData);
    connect(_ui->chartView, &ChartView::axisValuesChanged,
            this, &MainWindow::setAxisValues);
    connect(_ui->chartView, &ChartView::shiftLeft,
            this, &MainWindow::shiftLeft);
    connect(_ui->chartView, &ChartView::shiftRight,
            this, &MainWindow::shiftRight);
}

MainWindow::~MainWindow()
{
    if (_serialPort->isOpen())
        _serialPort->close();

    delete _serialPort;
    delete _ui;
}

void MainWindow::on_actionOpen_CSV_triggered()
{
    auto fileName = QFileDialog::getOpenFileName(this,
                                                 tr("Open CSV"),
                                                 QString(),
                                                 tr("CSV (*.csv)"));
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        _rawData = file.readAll();
        _ui->dataLog->setPlainText(_rawData);
        _rawLines = _rawData.split('\n');
        if (_rawLines.first().startsWith("ms"))
            _rawLines.removeFirst();
        _serialReader.process(_rawLines.mid(0, _serialReader.samples()));
        file.close();
    }
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
    _rawLines.clear();
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

void MainWindow::on_actionZoom_In_triggered()
{
    _ui->chartView->chart()->zoomIn();
    setAxisValues();
}

void MainWindow::on_actionZoom_Out_triggered()
{
    _ui->chartView->chart()->zoomOut();
    setAxisValues();
}

void MainWindow::on_actionReset_Zoom_triggered()
{
    _ui->chartView->chart()->zoomReset();
    setAxisValues();
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

void MainWindow::minXChanged(int value)
{
    if (value == _maxXSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum X must be != maximum X."));
    } else if (value > _maxXSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum X must be < maximum X."));
    } else {
        _ui->chartView->axisX()->setMin(value);
        _serialReader.setSamples(_maxXSpinBox->value()-_minXSpinBox->value());
    }
}

void MainWindow::maxXChanged(int value)
{
    if (value == _minXSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum X must be != maximum X."));
    } else if (value < _minXSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum X must be < maximum X."));
    } else {
        _ui->chartView->axisX()->setMax(value);
        _serialReader.setSamples(_maxXSpinBox->value()-_minXSpinBox->value());
    }
}

void MainWindow::minYChanged(int value)
{
    if (value == _maxYSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be != maximum Y."));
    } else if (value > _maxYSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be < maximum Y."));
    } else {
        _ui->chartView->axisY()->setMin(value);
    }
}

void MainWindow::maxYChanged(int value)
{
    if (value == _minYSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be != maximum Y."));
    } else if (value < _minYSpinBox->value()) {
        _ui->statusLog->appendPlainText(QString("Minimum Y must be < maximum Y."));
    } else {
        _ui->chartView->axisY()->setMax(value);
    }
}

void MainWindow::setupAxisX()
{
    _minXSpinBox = new QSpinBox(_ui->toolBar);
    _minXSpinBox->setRange(0, std::numeric_limits<int>::max());
    _minXSpinBox->setValue(0);
    auto minLabel = new QLabel("Minimum X: ", _ui->toolBar);
    minLabel->setMargin(6);
    _ui->toolBar->addWidget(minLabel);
    _ui->toolBar->addWidget(_minXSpinBox);
    _maxXSpinBox = new QSpinBox(_ui->toolBar);
    _maxXSpinBox->setRange(0, std::numeric_limits<int>::max());
    _maxXSpinBox->setValue(_serialReader.samples());
    auto maxLabel = new QLabel("Maximum X: ", _ui->toolBar);
    maxLabel->setMargin(6);
    _ui->toolBar->addWidget(maxLabel);
    _ui->toolBar->addWidget(_maxXSpinBox);
    connect(_minXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::minXChanged);
    connect(_maxXSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::maxXChanged);

    _ui->chartView->axisX()->setRange(_minXSpinBox->value(), _maxXSpinBox->value());

    _minXSpinBox->setEnabled(false);
    _maxXSpinBox->setEnabled(false);
}

void MainWindow::setupAxisY()
{
    _minYSpinBox = new QSpinBox(_ui->toolBar);
    _minYSpinBox->setRange(0, 1024);
    _minYSpinBox->setValue(200);
    auto minLabel = new QLabel("Minimum Y: ", _ui->toolBar);
    minLabel->setMargin(6);
    _ui->toolBar->addWidget(minLabel);
    _ui->toolBar->addWidget(_minYSpinBox);
    _maxYSpinBox = new QSpinBox(_ui->toolBar);
    _maxYSpinBox->setRange(0, 1024);
    _maxYSpinBox->setValue(300);
    auto maxLabel = new QLabel("Maximum Y: ", _ui->toolBar);
    maxLabel->setMargin(6);
    _ui->toolBar->addWidget(maxLabel);
    _ui->toolBar->addWidget(_maxYSpinBox);
    connect(_minYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::minYChanged);
    connect(_maxYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::maxYChanged);

    _ui->chartView->axisY()->setRange(_minYSpinBox->value(), _maxYSpinBox->value());
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

void MainWindow::setAxisValues()
{
    _minYSpinBox->setValue(static_cast<int>(_ui->chartView->axisY()->min()));
    _maxYSpinBox->setValue(static_cast<int>(_ui->chartView->axisY()->max()));
}

void MainWindow::shiftLeft()
{
    if (_rawLines.isEmpty())
        _rawLines = _rawData.split('\n');

    if (_minShift == 0) return;

    _minShift -= 10;
    if (!_maxShift)
        _maxShift -= _serialReader.samples() - 10;
    else
        _maxShift -= 10;

    auto lines = _rawLines.mid(_minShift, _maxShift);
    _serialReader.process(lines);
}

void MainWindow::shiftRight()
{
    if (_rawLines.isEmpty())
        _rawLines = _rawData.split('\n');

    if (_rawLines.size() <= _maxShift) return;

    _minShift += 10;
    if (!_maxShift)
        _maxShift += 10 + _serialReader.samples();
    else
        _maxShift += 10;

    auto lines = _rawLines.mid(_minShift, _maxShift);
    _serialReader.process(lines);
}
