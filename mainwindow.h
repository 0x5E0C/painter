#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMessageBox>
#include "process.h"
#include "public.h"
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    Ui::MainWindow *ui;
    process *processor;
    QVector<QPoint> line,sample_point,real_line;
    QTimer *timer=new QTimer(this);
    QTimer *send_timer = new QTimer(this);
    QList<QString> serialport_list;
    bool serial_state;
    QString  serial_choice;
    QSerialPort *serialport=new QSerialPort();
    QImage image;
    bool clear_flag=false;
    bool sample_flag=false;
    int real_length,real_width;
    bool reply_flag,autosend_flag;
    quint8 send_buffer[PACKET_LENGTH];
    void widgetInit();
    bool openSerialport();
    void closeSerialport();
    bool isInROI(QPoint pos);
    QPoint changeToRealPoint(QPoint p);

private slots:
    void changeSerialState();
    void searchSerialport();
    void recordSerialChoice(int choice);
    void readSerialport();
    void startSample();
    void sendData();
    void updateMapSize();
    void updateGraph();
    void writeSerialport();
};
#endif // MAINWINDOW_H
