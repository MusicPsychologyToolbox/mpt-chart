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

#include <QAudioDeviceInfo>
#include <QMap>
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class QActionGroup;
class QAudioRecorder;
class QDoubleSpinBox;
class QSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File
    void on_actionOpen_CSV_triggered();
    void on_actionExportCSV_triggered();
    void on_actionQuit_triggered();

    // Audio
    void on_audioMenu_aboutToShow();
    void audioInSelected(QAction *action);

    // Device
    void on_deviceMenu_aboutToShow();
    void deviceSelected(QAction *action);
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();

    // View
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionReset_Zoom_triggered();
    void on_actionPulse_triggered();

    // Help
    void on_actionAboutQt_triggered();

    // Other
    void showNewData(const QByteArray &data);
    void minXChanged(int value);
    void maxXChanged(int value);
    void minYChanged(int value);
    void maxYChanged(int value);
    void dpEpsilonChanged(double value);

    void setAxisValues();
    void recordAudio();
    void handleAudioInError();

private:
    void setupAxisX();
    void setupAxisY();
    void setupDpEpsilon();

    void setStandardBaudRates();
    void setSerialPortInfo();
    void setActionsForPortInfos();
    void setAudioDeviceInfo();
    void setActionsForAudioIn();

    QString documentsLocation() const;
    QString currentFileLocation() const;
    QString audioTargetFilePath(const QString &fileName);
    void createAudioDirectory();

    void appendLog(const QString &log);

private:
    Ui::MainWindow *_ui;

    QMenu *_baudMenu;
    QMenu *_portMenu;

    QActionGroup *_baudGroup;
    QActionGroup *_portGroup;
    QString _checkedDeviceAction;
    QMap<QString, QSerialPortInfo> _serialPortInfos;

    QMenu *_audioInMenu;
    QActionGroup *_audioInGroup;
    QMap<QString, QAudioDeviceInfo> _audioInInfos;
    QString _checkedAudioInAction;
    QAudioRecorder *_audioRecorder;

    QTimer _timer;
    const int _timer_msec = 50;

    SerialReader _serialReader;

    const int _initSize = 1024 * 1024 * 8; // 8MiB
    QByteArray _rawData;
    QList<QByteArray> _rawLines;

    QSpinBox *_minXSpinBox;
    QSpinBox *_maxXSpinBox;
    QSpinBox *_minYSpinBox;
    QSpinBox *_maxYSpinBox;
    QDoubleSpinBox *_dpEpsilonSpinBox;

    QString _currentSubDir;
};

#endif // MAINWINDOW_H
