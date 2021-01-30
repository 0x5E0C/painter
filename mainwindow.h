#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

#define canvas_length 500       //画板长
#define canvas_width  500       //画板宽
#define gap           50        //距离窗口边框的距离

#define OPENSTATE   0
#define CLOSESTATE  1

#define BASETIME    500         //自动搜索串口的时间间隔

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
    QVector<QPoint> line,sample_point,real_line;//鼠标轨迹，采样点，实际轨迹
    QTimer *timer=new QTimer(this);
    QList<QString> serialport_list;             //可用串口列表
    bool serial_state;                          //串口状态
    QString  serial_choice;                     //从串口下拉列表中选择的串口名
    QSerialPort *serialport=new QSerialPort();
    QImage image;
    bool clear_flag=false;                      //清除画板的标志位，true清除
    bool sample_flag=false;                     //画采样点的标志位，true画采样点
    int real_length,real_width;                 //画采样点的标志位，true画采样点
    void widgetInit();
    bool openSerialport();
    void closeSerialport();
    void writeSerialport(quint8 *data,int count);
    bool isInROI(QPoint pos);
    QPoint changeToRealPoint(QPoint p);
    QPoint changeToPainterPoint(QPoint p);

private slots:
    void changeSerialState();
    void searchSerialport();
    void recordSerialChoice(int choice);
    void readSerialport();
    void startSample();
    void syncCoordinateAxis();
    void sendData();
};
#endif // MAINWINDOW_H
