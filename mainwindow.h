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
    process *processor;                             //数据包处理器（新线程）
    QVector<QPoint> line,sample_point,real_line;    //鼠标轨迹点、采样点、车辆真实轨迹点
    QTimer *timer=new QTimer(this);                 //用于定时搜索串口的定时器
    QTimer *send_timer = new QTimer(this);          //用于定时发送数据的定时器
    QList<QString> serialport_list;                 //可用串口列表
    bool serial_state;                              //zigbee所在串口打开状态
    QString  serial_choice;                         //zigbee选择的串口名
    QSerialPort *serialport=new QSerialPort();      //初始化zigbee串口
    QImage image;                                   //初始化画板
    bool clear_flag=false;                          //清除轨迹标志位
    bool sample_flag=false;                         //采样标志
    int real_length,real_width;                     //真实场景的长、宽
    bool reply_flag,autosend_flag;                  //回复标志位、自动发送标志位
    quint8 send_buffer[PACKET_LENGTH];              //发送数据缓冲区
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
