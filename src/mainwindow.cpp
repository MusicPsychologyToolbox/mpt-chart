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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
    , _serialReader(this)
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
    setupDpEpsilon();

    _serialReader.setAxisX(_ui->chartView->axisX());
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
}

MainWindow::~MainWindow()
{
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
        _serialReader.load(_rawLines);
        file.close();
    } else {
        appendLog(QString("Error: Could not open CSV file %1.").arg(fileName));
    }
}

void MainWindow::on_actionExportCSV_triggered()
{
    auto fileName = QFileDialog::getSaveFileName(this,
                                                 tr("Export CSV"),
                                                 QString(),
                                                 tr("CSV (*.csv)"));
    if (!fileName.endsWith(".csv", Qt::CaseInsensitive))
        fileName.append(".csv");
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << "ms,sync,air1,air2,air3,pulse";
        stream << _rawData;
        file.close();
    } else {
        appendLog(QString("Error: Could not open export file %1.").arg(fileName));
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

void MainWindow::deviceSelected(QAction *action)
{
    if (action)
        _checkedAction = action->text();
}

void MainWindow::on_actionConnect_triggered()
{
    if (_serialReader.serialPort()->isOpen())
        return;

    _serialReader.clear();
    _rawData.clear();
    _rawLines.clear();
    _rawData.reserve(_initSize);
    _ui->dataLog->clear();
    on_actionReset_Zoom_triggered();

    auto baud = _baudGroup->checkedAction()->text().toInt();
    if (!_serialReader.serialPort()->setBaudRate(baud, QSerialPort::AllDirections)) {
        appendLog(QString("Error code: %1\n Message: %2")
                  .arg(_serialReader.serialPort()->error())
                  .arg(_serialReader.serialPort()->errorString()));
        return;
    }

    auto action = _portGroup->checkedAction();
    if (!action) return;
    auto portInfo = _serialPortInfos.value(action->text());
    appendLog(QString("Configured Baud (%1) and Port (%2).")
              .arg(baud).arg(portInfo.portName()));
    if (portInfo.isNull()) {
        appendLog("ERROR: no valid serial port found!");
        return;
    } else {
        _serialReader.serialPort()->setPort(portInfo);
    }

    appendLog(QString("Start reading data. Baud: %1 Port: %2")
              .arg(_serialReader.serialPort()->baudRate())
              .arg(_serialReader.serialPort()->portName()));
    if (!_serialReader.serialPort()->open(QIODevice::ReadOnly)) {
        appendLog(QString("Failed to open port %1 error: %2")
                  .arg(_serialReader.serialPort()->portName())
                  .arg(_serialReader.serialPort()->errorString()));
        return;
    }

    // Required on Windows; otherwise the Arduion restart does not happen
    _serialReader.serialPort()->setRequestToSend(true);
    _serialReader.serialPort()->setDataTerminalReady(true);

    _timer.start(_timer_msec);
}

void MainWindow::on_actionDisconnect_triggered()
{
    if (!_serialReader.serialPort()->isOpen())
        return;
    _serialReader.serialPort()->close();
    _timer.stop();
    appendLog("Data recoding stopped.");
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
    _rawData.append('\n');
    _rawData.append(data);
    _ui->dataLog->appendPlainText(data);
}

void MainWindow::minXChanged(int value)
{
    if (value == _maxXSpinBox->value()) {
        appendLog("Minimum X must be != maximum X.");
    } else if (value > _maxXSpinBox->value()) {
        appendLog("Minimum X must be < maximum X.");
    } else {
        _ui->chartView->axisX()->setMin(value);
        _serialReader.setSamples(_maxXSpinBox->value()-_minXSpinBox->value());
    }
}

void MainWindow::maxXChanged(int value)
{
    if (value == _minXSpinBox->value()) {
        appendLog("Minimum X must be != maximum X.");
    } else if (value < _minXSpinBox->value()) {
        appendLog("Minimum X must be < maximum X.");
    } else {
        _ui->chartView->axisX()->setMax(value);
        _serialReader.setSamples(_maxXSpinBox->value()-_minXSpinBox->value());
    }
}

void MainWindow::minYChanged(int value)
{
    if (value == _maxYSpinBox->value()) {
        appendLog("Minimum Y must be != maximum Y.");
    } else if (value > _maxYSpinBox->value()) {
        appendLog("Minimum Y must be < maximum Y.");
    } else {
        _ui->chartView->axisY()->setMin(value);
    }
}

void MainWindow::maxYChanged(int value)
{
    if (value == _minYSpinBox->value()) {
        appendLog("Minimum Y must be != maximum Y.");
    } else if (value < _minYSpinBox->value()) {
        appendLog("Minimum Y must be < maximum Y.");
    } else {
        _ui->chartView->axisY()->setMax(value);
    }
}

void MainWindow::dpEpsilonChanged(double value)
{
    _serialReader.setDpEpsilon(value);

    if (!_serialReader.serialPort()->isOpen())
        _serialReader.reload();
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
    _maxYSpinBox->setValue(350);
    auto maxLabel = new QLabel("Maximum Y: ", _ui->toolBar);
    maxLabel->setMargin(6);

    _ui->toolBar->addWidget(maxLabel);
    _ui->toolBar->addWidget(_maxYSpinBox);

    connect(_minYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::minYChanged);
    connect(_maxYSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::maxYChanged);

    _ui->chartView->axisY()->setRange(_minYSpinBox->value(), _maxYSpinBox->value());
}

void MainWindow::setupDpEpsilon()
{
    auto label = new QLabel("DP Epsilon: ", _ui->toolBar);
    label->setMargin(6);
    _dpEpsilonSpinBox = new QDoubleSpinBox(_ui->toolBar);
    _dpEpsilonSpinBox->setRange(0.0, 100.0);
    _dpEpsilonSpinBox->setDecimals(1);
    _dpEpsilonSpinBox->setSingleStep(0.1);
    _dpEpsilonSpinBox->setValue(_serialReader.dpEpsilon());

    _ui->toolBar->addWidget(label);
    _ui->toolBar->addWidget(_dpEpsilonSpinBox);

    connect(_dpEpsilonSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::dpEpsilonChanged);
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
    connect(_portGroup, &QActionGroup::triggered, this, &MainWindow::deviceSelected);
    _portGroup->setExclusive(true);
    setActionsForPortInfos();
    _ui->deviceMenu->addMenu(_portMenu);
}

void MainWindow::setActionsForPortInfos()
{
    for (auto portInfo: QSerialPortInfo::availablePorts()) {
        auto port = new QAction(portInfo.portName(), _portMenu);
        port->setCheckable(true);
        if (port->text() == _checkedAction)
            port->setChecked(true);
        _serialPortInfos[portInfo.portName()] = portInfo;
        _portMenu->addAction(port);
        _portGroup->addAction(port);
    }

    if (!_portGroup->checkedAction() && _portGroup->actions().size())
        _portGroup->actions().first()->setChecked(true);
}

void MainWindow::appendLog(const QString &log) {
    _ui->statusLog->appendPlainText(log);
}

void MainWindow::setAxisValues()
{
    _minYSpinBox->setValue(static_cast<int>(_ui->chartView->axisY()->min()));
    _maxYSpinBox->setValue(static_cast<int>(_ui->chartView->axisY()->max()));
}
