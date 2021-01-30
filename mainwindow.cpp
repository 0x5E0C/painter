#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    widgetInit();
    connect(timer,SIGNAL(timeout()),this,SLOT(searchSerialport()));
    connect(ui->openButton,SIGNAL(clicked()),this,SLOT(changeSerialState()));
    connect(ui->serialportBox,SIGNAL(activated(int)),this,SLOT(recordSerialChoice(int)));
    connect(serialport,SIGNAL(readyRead()),this,SLOT(readSerialport()));
    connect(ui->sampleButton,SIGNAL(clicked()),this,SLOT(startSample()));
    connect(ui->syncButton,SIGNAL(clicked()),this,SLOT(syncCoordinateAxis()));
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendData()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*函数名：widgetInit*/
/*参数：无*/
/*功能：初始化软件界面与全局变量*/
/*返回值：无*/
void MainWindow::widgetInit()
{
    image=QImage(canvas_length,canvas_width,QImage::Format_RGB32);
    image.fill(qRgb(255,255,255));                          //画板填充为白色
    serial_state=CLOSESTATE;
    serial_choice="";
    ui->openButton->setIcon(QPixmap(":/state/close.png"));  //按钮图标设置为关闭图标
    ui->openButton->setText("打开串口");
    timer->start(BASETIME);
}

/*函数名：changeSerialState*/
/*参数：无*/
/*功能：按下“打开串口”按钮后改变按钮显示与串口状态变量*/
/*返回值：无*/
void MainWindow::changeSerialState()
{
    if(serial_state==CLOSESTATE && openSerialport())
    {
        ui->openButton->setIcon(QPixmap(":/state/open.png"));
        ui->openButton->setText("关闭串口");
        serial_state=OPENSTATE;
    }
    else if(serial_state==OPENSTATE)
    {
        ui->openButton->setIcon(QPixmap(":/state/close.png"));
        ui->openButton->setText("打开串口");
        serial_state=CLOSESTATE;
        closeSerialport();
    }
}

/*函数名：recordSerialChoice*/
/*参数：串口下拉选择列表的选择*/
/*功能：记录和在更换串口选择时重启串口*/
/*返回值：无*/
void MainWindow::recordSerialChoice(int choice)
{
    serial_choice=ui->serialportBox->itemText(choice);      //获取从串口下拉列表选择的串口名
    if(serial_state==OPENSTATE)
    {
        closeSerialport();
        openSerialport();
    }
}

/*函数名：searchSerialport*/
/*参数：无*/
/*功能：搜索并更新串口下拉列表*/
/*返回值：无*/
void MainWindow::searchSerialport()
{
    QList<QString> temp_list;
    if(QSerialPortInfo::availablePorts().size()!=0)
    {
        if(ui->serialportBox->itemText(0)=="")
        {
            ui->serialportBox->removeItem(0);       //有可用串口时，移除空白选项
        }
        if(serialport_list.isEmpty())
        {
            foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
            {
                serialport_list.append(info.portName());
            }
            for(int i=0;i<serialport_list.size();i++)
            {
                ui->serialportBox->addItem(serialport_list[i]); //第一次搜索串口全部更新到下拉列表
            }
        }
        else
        {
            foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
            {
                temp_list.append(info.portName());
            }
            if(temp_list!=serialport_list)
            {
                ui->serialportBox->clear();
                for(int i=0;i<temp_list.size();i++)
                {
                    ui->serialportBox->addItem(temp_list[i]); //与上一次搜索结果对比，不同时更新到下拉列表
                }
                serialport_list=temp_list;
            }
        }
        ui->serialportBox->setCurrentText(serial_choice);
    }
    else
    {
        ui->serialportBox->clear();
        ui->serialportBox->addItem("");        //下拉列表无可用串口，添加空白选项
        serialport_list.clear();
    }
}

/*函数名：openSerialport*/
/*参数：无*/
/*功能：打开串口*/
/*返回值：true：打开串口成功  false：打开串口失败*/
bool MainWindow::openSerialport()
{
    serialport->setPortName(ui->serialportBox->currentText());
    serialport->setBaudRate(ui->baudrateBox->currentText().toInt());
    serialport->setStopBits(QSerialPort::OneStop);
    serialport->setFlowControl(QSerialPort::NoFlowControl);
    serialport->setDataBits(QSerialPort::Data8);
    serialport->setParity(QSerialPort::NoParity);
    QSerialPortInfo serial_info(ui->serialportBox->currentText());
    if(serial_info.isBusy()==true)
    {
        QString dlgTitle="错误";
        QString strInfo="打开串口失败!串口已被占用!";
        QMessageBox::critical(this,dlgTitle,strInfo);
        serial_state=OPENSTATE;
        changeSerialState();
        return false;
    }
    if(!serialport->open(QIODevice::ReadWrite))
    {
        QString dlgTitle="错误";
        QString strInfo="打开串口失败!";
        QMessageBox::critical(this,dlgTitle,strInfo);
        return false;
    }
    return true;
}

/*函数名：closeSerialport*/
/*参数：无*/
/*功能：关闭串口*/
/*返回值：无*/
void MainWindow::closeSerialport()
{
    serialport->close();
}

/*函数名：writeSerialport*/
/*参数：*data：传输数据流首地址的指针  count：传输数据长度*/
/*功能：串口传输数据*/
/*返回值：无*/
void MainWindow::writeSerialport(quint8 *data,int count)
{
    QByteArray buffer;
    buffer.resize(count);
    memcpy(buffer.data(),data,count);
    serialport->write(buffer);
}

/*函数名：readSerialport*/
/*参数：无*/
/*功能：读取并处理串口数据*/
/*返回值：无*/
/*数据格式：id x坐标符号位 x坐标高8位 x坐标低8位 y坐标符号位 y坐标高8位 y坐标低8位 校验和高8位 校验和低8位*/
/*         0      1         2         3         4          5         6         7         8    */
/*符号位：0为正数，1为负数*/
void MainWindow::readSerialport()
{
    quint8 *rec_buffer=(quint8*)serialport->readAll().data();
    int sum=0;
    for(int i=0;i<=6;i++)
    {
        sum+=rec_buffer[i];
    }
    if(sum!=(rec_buffer[7]<<8|rec_buffer[8]))
    {
        return;
    }
    int x=(rec_buffer[1]==0x00?rec_buffer[2]<<8|rec_buffer[3]:-(rec_buffer[2]<<8|rec_buffer[3]));
    int y=(rec_buffer[4]==0x00?rec_buffer[5]<<8|rec_buffer[6]:-(rec_buffer[5]<<8|rec_buffer[6]));
    if(rec_buffer[0]==0x00)
    {
        real_length=x;
        real_width=y;
        ui->xlabel->setText(QString::number(x));
        ui->ylabel->setText(QString::number(y));
    }
    else
    {
        real_line.append(changeToPainterPoint(QPoint(x,y)));
        update();
    }
}

/*函数名：mousePressEvent*/
/*参数：鼠标事件*/
/*功能：鼠标左键按下时开始画图，右键清除画图轨迹*/
/*返回值：无*/
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton && isInROI(event->pos()))
    {
        clear_flag=false;
        line.append(event->pos());      //记录坐标
        update();
    }
    else if(event->button()==Qt::RightButton && isInROI(event->pos()))
    {
        clear_flag=true;
        sample_flag=false;
        image.fill(qRgb(255,255,255));  //清除画板
        line.clear();
        sample_point.clear();
        real_line.clear();
        update();
    }
}

/*函数名：mouseMoveEvent*/
/*参数：鼠标事件*/
/*功能：按下鼠标左键并移动鼠标时画出轨迹*/
/*返回值：无*/
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isInROI(event->pos()))
    {
        line.append(event->pos());      //记录坐标
        update();
    }
}

/*函数名：mouseReleaseEvent*/
/*参数：鼠标事件*/
/*功能：松开鼠标结束画图并显示组成轨迹的点的数量*/
/*返回值：无*/
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(clear_flag==false && isInROI(event->pos()))
    {
        line.append(event->pos());      //记录坐标
        update();
    }
    ui->showLabel->setText(QString::number(line.size()));//显示轨迹点的数量
}

/*函数名：paintEvent*/
/*参数：画图事件*/
/*功能：画出鼠标轨迹和返回坐标组成的轨迹*/
/*返回值：无*/
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawImage(gap,gap,image);
    painter.setPen(QPen(Qt::green,3));
    for(int i=0;i<line.size()-1;i++)
    {
        painter.drawLine(line.at(i),line.at(i+1));  //画出鼠标轨迹
    }
    if(sample_flag)
    {
        painter.setPen(QPen(Qt::red,6));
        for(int i=0;i<sample_point.size();i++)
        {
            painter.drawPoint(sample_point.at(i));  //画出采样点
        }
    }
    if(!real_line.isEmpty())
    {
        painter.setPen(QPen(Qt::blue,3));
        for(int i=0;i<real_line.size()-1;i++)
        {
            painter.drawLine(real_line.at(i),real_line.at(i+1));//画出实际轨迹
        }
    }
}

/*函数名：isInROI*/
/*参数：QPoint类型的点*/
/*功能：检测传入的点的坐标是否在画板内*/
/*返回值：true：在画板内  false：在画板外*/
bool MainWindow::isInROI(QPoint pos)
{
    return (pos.x()>gap && pos.x()<(gap+canvas_length) &&
            pos.y()>gap && pos.y()<(gap+canvas_width));
}

/*函数名：startSample*/
/*参数：无*/
/*功能：对鼠标轨迹进行采样*/
/*返回值：无*/
void MainWindow::startSample()
{
    if(ui->editFrequency->text().isEmpty())
    {
        QString dlgTitle="错误";
        QString strInfo="采样错误!";
        QMessageBox::critical(this,dlgTitle,strInfo);
        return;
    }
    int frequency;
    int j=0;
    sample_flag=true;
    frequency=ui->editFrequency->text().toInt();
    for(int i=0;i<line.size();i=j*frequency)
    {
        QPoint point=line.at(i);
        sample_point.append(point);
        j++;
    }
    sample_point.append(line.last());
    update();
}

/*函数名：syncCoordinateAxis*/
/*参数：无*/
/*功能：请求同步实际坐标数据*/
/*返回值：无*/
void MainWindow::syncCoordinateAxis()
{
    quint8 buffer[9]={0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x05,0xFA};
    writeSerialport(buffer,sizeof(buffer)/sizeof(quint8));
}

/*函数名：sendData*/
/*参数：无*/
/*功能：发送采样完成的点的坐标数据*/
/*返回值：无*/
/*数据格式：命令 x坐标符号位 x坐标高8位 x坐标低8位 y坐标符号位 y坐标高8位 y坐标低8位 校验和高8位 校验和低8位*/
/*         0      1         2         3         4          5         6         7         8    */
/*符号位：0为正数，1为负数*/
void MainWindow::sendData()
{
    quint8 buffer[9];
    int sum=0;
    if(ui->xlabel->text().isEmpty() || ui->ylabel->text().isEmpty())
    {
        QString dlgTitle="错误";
        QString strInfo="未同步坐标系!";
        QMessageBox::critical(this,dlgTitle,strInfo);
        return;
    }
    buffer[0]=0x01;
    for(int i=0;i<sample_point.size()-1;i++)
    {
        buffer[1]=(changeToRealPoint(sample_point.at(i)).x()>=0)?(0x00):(0x01);
        buffer[2]=changeToRealPoint(sample_point.at(i)).x()/256;
        buffer[3]=changeToRealPoint(sample_point.at(i)).x()%256;
        buffer[4]=(changeToRealPoint(sample_point.at(i)).y()>=0)?(0x00):(0x01);
        buffer[5]=changeToRealPoint(sample_point.at(i)).y()/256;
        buffer[6]=changeToRealPoint(sample_point.at(i)).y()%256;
        for(int j=0;j<9;j++)
        {
            sum+=buffer[j];
        }
        buffer[7]=sum/255;
        buffer[8]=sum%256;
        writeSerialport(buffer,sizeof(buffer)/sizeof(quint8));
    }
}

/*函数名：changeToRealPoint*/
/*参数：QPoint类型的画板中的点*/
/*功能：将画板中的点的坐标转化为实际点的坐标*/
/*返回值：QPoint类型的实际的点*/
QPoint MainWindow::changeToRealPoint(QPoint p)
{
    int x,y;
    x=(p.x()-gap)*real_length/canvas_length;
    y=(canvas_width-p.y()+gap)*real_width/canvas_width;
    return QPoint(x,y);
}

/*函数名：changeToPainterPoint*/
/*参数：QPoint类型的实际的点*/
/*功能：将实际点的坐标转化为画板中的点的坐标*/
/*返回值：QPoint类型的画板中的点*/
QPoint MainWindow::changeToPainterPoint(QPoint p)
{
    int x,y;
    x=p.x()*canvas_length/real_length+gap;
    y=canvas_width-p.y()*canvas_width/real_width+gap;
    return QPoint(x,y);
}
