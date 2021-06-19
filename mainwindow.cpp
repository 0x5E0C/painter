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
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendData()));
    connect(ui->updateMapButton,SIGNAL(clicked()),this,SLOT(updateMapSize()));
    connect(processor,SIGNAL(processed()),this,SLOT(updateGraph()));
    connect(send_timer,SIGNAL(timeout()),this,SLOT(writeSerialport()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
函数名：widgetInit
参数：无
返回值：无
功能：初始化界面显示和变量
*/
void MainWindow::widgetInit()
{
    image=QImage(canvas_length,canvas_width,QImage::Format_RGB32);  //设置画板大小
    image.fill(qRgb(255,255,255));                                  //画板填充为白色
    serial_state=CLOSESTATE;                                        //初始化选择的串口名为空
    serial_choice="";                                               //初始化串口状态为关闭
    reply_flag=false;                                               //初始化回复标志位为false
    autosend_flag=false;                                            //初始化自动发送标志位为false
    processor = new process();
    processor->setReplyFlag(&reply_flag);                           //传入回复标志位的地址给处理数据的线程
    ui->openButton->setIcon(QPixmap(":/state/close.png"));          //按钮图标设置为关闭图标
    ui->openButton->setText("打开串口");                             //设置按钮文本
    timer->start(BASETIME);                                         //开始定时搜索可用串口
}

/*
函数名：changeZigbeeSerialState
参数：无
返回值：无
功能：按下打开/关闭zigbee串口按钮时改变按钮显示
*/
void MainWindow::changeSerialState()
{
    if(serial_state==CLOSESTATE && openSerialport())
    { 
        ui->openButton->setIcon(QPixmap(":/state/open.png"));       //改变按钮贴图和文本
        ui->openButton->setText("关闭串口");
        serial_state=OPENSTATE;
    }
    else if(serial_state==OPENSTATE)
    {
        ui->openButton->setIcon(QPixmap(":/state/close.png"));      //改变按钮贴图和文本
        ui->openButton->setText("打开串口");
        serial_state=CLOSESTATE;
        closeSerialport();
    }
}

/*
函数名：recordZigbeeSerialChoice
参数：无
返回值：无
功能：记录zigbee串口下拉列表的选择
*/
void MainWindow::recordSerialChoice(int choice)
{
    serial_choice=ui->serialportBox->itemText(choice);              //获取从串口下拉列表选择的串口名
    if(serial_state==OPENSTATE)
    {
        closeSerialport();                                          //重启串口
        openSerialport();
    }
}

/*
函数名：searchSerialport
参数：无
返回值：无
功能：寻找可用串口并显示
*/
void MainWindow::searchSerialport()
{
    QList<QString> temp_list;
    if(QSerialPortInfo::availablePorts().size()!=0)                 //可用串口非空
    {
        if(ui->serialportBox->itemText(0)=="")                      //原先可用串口列表只包含空选项
        {
            ui->serialportBox->removeItem(0);                       //清除空选项
        }
        if(serialport_list.isEmpty())                               //之前记录的串口列表为空
        {
            foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
            {
                serialport_list.append(info.portName());            //将所有串口添加进列表
            }
            for(int i=0;i<serialport_list.size();i++)
            {
                ui->serialportBox->addItem(serialport_list[i]);     //显示所有串口到串口列表
            }
        }
        else
        {
            foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
            {
                temp_list.append(info.portName());                  // 记录所有串口
            }
            if(temp_list!=serialport_list)                          //可用串口有变化
            {
                ui->serialportBox->clear();                         //清除下拉框所有选项
                for(int i=0;i<temp_list.size();i++)
                {
                    ui->serialportBox->addItem(temp_list[i]);       //重新添加下拉框选项
                }
                serialport_list=temp_list;                          //储存本次搜寻结果
            }
        }
        ui->serialportBox->setCurrentText(serial_choice);           //显示当前串口选择
    }
    else
    {
        ui->serialportBox->clear();                                 //没有可用串口时清除下拉框所有选项
        ui->serialportBox->addItem("");                             //添加一个空选项
        serialport_list.clear();                                    //记录本次搜寻结果为空
    }
}

/*
函数名：openZigbeeSerialport
参数：无
返回值：true：打开成功
       false：打开失败
功能：打开zigbee所在串口
*/
bool MainWindow::openSerialport()
{
    serialport->setPortName(ui->serialportBox->currentText());      //设置串口名
    serialport->setBaudRate(ui->baudrateBox->currentText().toInt());//设置波特率
    serialport->setStopBits(QSerialPort::OneStop);                  //设置停止位
    serialport->setFlowControl(QSerialPort::NoFlowControl);         //设置控制流
    serialport->setDataBits(QSerialPort::Data8);                    //设置数据位
    serialport->setParity(QSerialPort::NoParity);                   //设置检验位
    QSerialPortInfo serial_info(ui->serialportBox->currentText());  //实例化串口
    if(serial_info.isBusy()==true)                                  //串口正忙
    {
        QString dlgTitle="错误";
        QString strInfo="打开串口失败!串口已被占用!";
        QMessageBox::critical(this,dlgTitle,strInfo);               //弹出错误提示窗口
        serial_state=OPENSTATE;
        changeSerialState();
        return false;
    }
    if(!serialport->open(QIODevice::ReadWrite))                     //串口无法读写
    {
        QString dlgTitle="错误";
        QString strInfo="打开串口失败!";
        QMessageBox::critical(this,dlgTitle,strInfo);
        return false;
    }
    return true;
}

/*
函数名：closeZigbeeSerialport
参数：无
返回值：无
功能：关闭zigbee所在串口
*/
void MainWindow::closeSerialport()
{
    serialport->close();
}

/*
函数名：writeZigbeeSerialport
参数：无
返回值：无
功能：向zigbee发送数据
*/
void MainWindow::writeSerialport()
{
    QByteArray buffer;
    buffer.resize(PACKET_LENGTH);
    memcpy(buffer.data(),send_buffer,PACKET_LENGTH*sizeof(quint8));
    serialport->write(buffer);                                      //写数据到串口
}

/*
函数名：readZigbeeSerialport
参数：无
返回值：无
功能：读zigbee的数据
*/
void MainWindow::readSerialport()
{
    QByteArray src_data=serialport->readAll();                      //读串口数据
    processor->setCacheData(src_data);                              //将数据传入待处理区域
    if(!processor->isbusy)                                          //线程空闲
    {
        processor->start();                                         //开启新线程处理数据
    }
}

/*
函数名：mousePressEvent
参数：鼠标事件
返回值：无
功能：qt内置函数，鼠标按钮按下事件
*/
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton && isInROI(event->pos()))    //鼠标左键按下
    {
        clear_flag=false;                                           
        line.append(event->pos());                                  //记录坐标
        update();                                                   //更新画板
    }
    else if(event->button()==Qt::RightButton && isInROI(event->pos()))//鼠标右键按下
    {
        clear_flag=true;                                            
        sample_flag=false;
        image.fill(qRgb(255,255,255));                              //清除画板
        line.clear();                                               //清除画线数据
        sample_point.clear();                                       //清除采样点
        real_line.clear();                                          //清除小车轨迹
        update();                                                   //更新画板
    }
}

/*
函数名：mouseMoveEvent
参数：鼠标事件
返回值：无
功能：qt内置函数，鼠标移动事件
*/
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isInROI(event->pos()))
    {
        line.append(event->pos());                                  //记录鼠标坐标
        update();
    }
}

/*
函数名：mouseReleaseEvent
参数：鼠标事件
返回值：无
功能：qt内置函数，鼠标按钮释放事件
*/
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(clear_flag==false && isInROI(event->pos()))
    {
        line.append(event->pos());                                  //记录鼠标坐标
        update();
    }
    ui->showLabel->setText(QString::number(line.size()));           //显示轨迹点的数量
}

/*
函数名：paintEvent
参数：画图事件
返回值：无
功能：qt内置函数，画图事件
*/
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawImage(gap,gap,image);                               //设置画板起始坐标
    painter.setPen(QPen(Qt::green,3));                              //设置画笔颜色
    for(int i=0;i<line.size()-1;i++)
    {
        painter.drawLine(line.at(i),line.at(i+1));                  //画出鼠标轨迹
    }
    if(sample_flag)
    {
        painter.setPen(QPen(Qt::red,6));
        for(int i=0;i<sample_point.size();i++)
        {
            painter.drawPoint(sample_point.at(i));                  //画出采样点
        }
    }
    if(!real_line.isEmpty())
    {
        painter.setPen(QPen(Qt::blue,3));
        for(int i=0;i<real_line.size()-1;i++)
        {
            painter.drawLine(real_line.at(i),real_line.at(i+1));    //画出小车轨迹
        }
    }
}

/*
函数名：isInROI
参数：坐标
返回值：true：在画板范围内
       false：在画板范围外
功能：判断该坐标在画板内还是画板外
*/
bool MainWindow::isInROI(QPoint pos)
{
    return (pos.x()>gap && pos.x()<(gap+canvas_length) &&
            pos.y()>gap && pos.y()<(gap+canvas_width));
}

/*
函数名：startSample
参数：坐标
返回值：无
功能：开始采样
*/
void MainWindow::startSample()
{
    if(ui->editFrequency->text().isEmpty())                         //未输入采样频率
    {
        QString dlgTitle="错误";
        QString strInfo="采样错误!";
        QMessageBox::critical(this,dlgTitle,strInfo);               //弹出错误弹窗
        return;
    }
    int frequency;
    int j=0;
    sample_flag=true;
    frequency=ui->editFrequency->text().toInt();
    for(int i=0;i<line.size();i=j*frequency)
    {
        QPoint point=line.at(i);
        sample_point.append(point);                                 //采样
        j++;
    }
    sample_point.append(line.last());                               //添加最后一个点
    update();
}

/*
函数名：sendData
参数：无
返回值：无
功能：发送数据给下位机
*/
void MainWindow::sendData()
{
    quint8 buffer[PACKET_LENGTH];
    int sum=0;
    if(ui->xlabel->text().isEmpty() || ui->ylabel->text().isEmpty())//未输入地图大小
    {
        QString dlgTitle="错误";
        QString strInfo="未同步坐标系!";
        QMessageBox::critical(this,dlgTitle,strInfo);               //错误弹窗
        return;
    }
    for(int i=0;i<sample_point.size()-2;i++)
    {
        buffer[0]=0x5E;                                             //包头
        buffer[1]=0x0C;
        buffer[2]=car_id;                                           
        buffer[3]=TASK_CMD;                                         //任务指令
        buffer[4]=(changeToRealPoint(sample_point.at(i)).x()&0xFF00)>>8;//采样点转为真实坐标后x坐标的高八位
        buffer[5]=changeToRealPoint(sample_point.at(i)).x()&0x00FF;     //采样点转为真实坐标后x坐标的低八位
        buffer[6]=(changeToRealPoint(sample_point.at(i)).y()&0xFF00)>>8;//采样点转为真实坐标后y坐标的高八位
        buffer[7]=changeToRealPoint(sample_point.at(i)).y()&0x00FF;     //采样点转为真实坐标后y坐标的低八位
        sum=0;
        for(int j=0;j<8;j++)
        {
            sum+=buffer[j];                                             //计算校验和
        }
        buffer[8]=(sum&0xFF00)>>8;                                      //检验和高八位
        buffer[9]=sum&0x00FF;
        qDebug()<<"next ";
        memcpy(send_buffer,buffer,PACKET_LENGTH*sizeof(quint8));        //传入发送缓冲区
        writeSerialport();                                              //发送
        send_timer->start(SENDTIME);                                    //自动发送
        autosend_flag=true;
        while(!reply_flag)
        {
            QCoreApplication::processEvents();                          //事件循环
        }
        send_timer->stop();                                             //停止发送
        autosend_flag=false;
        reply_flag=false;
    }
    buffer[0]=0x5E;
    buffer[1]=0x0C;
    buffer[2]=car_id;
    buffer[3]=FINISH_CMD;                                               //完成指令
    buffer[4]=(changeToRealPoint(sample_point.at(sample_point.size()-1)).x()&0xFF00)>>8;//最后一个点x坐标高8位
    buffer[5]=changeToRealPoint(sample_point.at(sample_point.size()-1)).x()&0x00FF;     //最后一个点x坐标低8位
    buffer[6]=(changeToRealPoint(sample_point.at(sample_point.size()-1)).y()&0xFF00)>>8;//最后一个点y坐标高8位
    buffer[7]=changeToRealPoint(sample_point.at(sample_point.size()-1)).y()&0x00FF;     //最后一个点y坐标低8位
    sum=0;
    for(int j=0;j<8;j++)
    {
        sum+=buffer[j];
    }
    buffer[8]=(sum&0xFF00)>>8;
    buffer[9]=sum&0x00FF;
    memcpy(send_buffer,buffer,PACKET_LENGTH*sizeof(quint8));
    writeSerialport();
    send_timer->start(SENDTIME);
    autosend_flag=true;
    while(!reply_flag)
    {
        QCoreApplication::processEvents();
    }
    send_timer->stop();
    autosend_flag=false;
    reply_flag=false;
    qDebug()<<"finish";
}

/*
函数名：changeToRealPoint
参数：画板内的坐标点
返回值：实际场景的坐标点
功能：将画板内的坐标点转化为实际场景的坐标点
*/
QPoint MainWindow::changeToRealPoint(QPoint p)
{
    int x,y;
    x=(p.x()-gap)*real_length/canvas_length;
    y=(canvas_width-p.y()+gap)*real_width/canvas_width;
    return QPoint(x,y);
}

/*
函数名：updateMapSize
参数：无
返回值：无
功能：更新地图大小
*/
void MainWindow::updateMapSize()
{
    if(ui->editMapX->text().isEmpty() || ui->editMapY->text().isEmpty())//未填写实际场景大小
    {
        QString dlgTitle="错误";
        QString strInfo="请先填写地图信息!";
        QMessageBox::critical(this,dlgTitle,strInfo);                   //错误弹窗
        return;
    }
    int x=ui->editMapX->text().toInt();
    int y=ui->editMapY->text().toInt();
    real_length=x;
    real_width=y;
    ui->xlabel->setText(QString::number(x));                            //设置x坐标轴长度标识
    ui->ylabel->setText(QString::number(y));                            //设置y坐标轴长度标识
    processor->setMapSize(real_length,real_width);
}

/*
函数名：updateMapSize
参数：无
返回值：无
功能：更新地图大小
*/
void MainWindow::updateGraph()
{
    real_line.append(processor->line_data);                             //添加小车返回的轨迹点
    update();
}
