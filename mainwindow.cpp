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

void MainWindow::widgetInit()
{
    image=QImage(canvas_length,canvas_width,QImage::Format_RGB32);
    image.fill(qRgb(255,255,255));                          //画板填充为白色
    serial_state=CLOSESTATE;
    serial_choice="";
    reply_flag=false;
    autosend_flag=false;
    processor = new process();
    processor->setReplyFlag(&reply_flag);
    ui->openButton->setIcon(QPixmap(":/state/close.png"));  //按钮图标设置为关闭图标
    ui->openButton->setText("打开串口");
    timer->start(BASETIME);
}

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

void MainWindow::recordSerialChoice(int choice)
{
    serial_choice=ui->serialportBox->itemText(choice);      //获取从串口下拉列表选择的串口名
    if(serial_state==OPENSTATE)
    {
        closeSerialport();
        openSerialport();
    }
}

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

void MainWindow::closeSerialport()
{
    serialport->close();
}

void MainWindow::writeSerialport()
{
    QByteArray buffer;
    buffer.resize(PACKET_LENGTH);
    memcpy(buffer.data(),send_buffer,PACKET_LENGTH*sizeof(quint8));
    serialport->write(buffer);
}

void MainWindow::readSerialport()
{
    QByteArray src_data=serialport->readAll();
    processor->setCacheData(src_data);
    if(!processor->isbusy)
    {
        processor->start();
    }
}

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

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isInROI(event->pos()))
    {
        line.append(event->pos());      //记录坐标
        update();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(clear_flag==false && isInROI(event->pos()))
    {
        line.append(event->pos());      //记录坐标
        update();
    }
    ui->showLabel->setText(QString::number(line.size()));//显示轨迹点的数量
}

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

bool MainWindow::isInROI(QPoint pos)
{
    return (pos.x()>gap && pos.x()<(gap+canvas_length) &&
            pos.y()>gap && pos.y()<(gap+canvas_width));
}

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

void MainWindow::sendData()
{
    quint8 buffer[PACKET_LENGTH];
    int sum=0;
    if(ui->xlabel->text().isEmpty() || ui->ylabel->text().isEmpty())
    {
        QString dlgTitle="错误";
        QString strInfo="未同步坐标系!";
        QMessageBox::critical(this,dlgTitle,strInfo);
        return;
    }
    for(int i=0;i<sample_point.size()-2;i++)
    {
        buffer[0]=0x5E;
        buffer[1]=0x0C;
        buffer[2]=car_id;
        buffer[3]=TASK_CMD;
        buffer[4]=(changeToRealPoint(sample_point.at(i)).x()&0xFF00)>>8;
        buffer[5]=changeToRealPoint(sample_point.at(i)).x()&0x00FF;
        buffer[6]=(changeToRealPoint(sample_point.at(i)).y()&0xFF00)>>8;
        buffer[7]=changeToRealPoint(sample_point.at(i)).y()&0x00FF;
        sum=0;
        for(int j=0;j<8;j++)
        {
            sum+=buffer[j];
        }
        buffer[8]=(sum&0xFF00)>>8;
        buffer[9]=sum&0x00FF;
        qDebug()<<"next ";
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
    }
    buffer[0]=0x5E;
    buffer[1]=0x0C;
    buffer[2]=car_id;
    buffer[3]=FINISH_CMD;
    buffer[4]=(changeToRealPoint(sample_point.at(sample_point.size()-1)).x()&0xFF00)>>8;
    buffer[5]=changeToRealPoint(sample_point.at(sample_point.size()-1)).x()&0x00FF;
    buffer[6]=(changeToRealPoint(sample_point.at(sample_point.size()-1)).y()&0xFF00)>>8;
    buffer[7]=changeToRealPoint(sample_point.at(sample_point.size()-1)).y()&0x00FF;
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

QPoint MainWindow::changeToRealPoint(QPoint p)
{
    int x,y;
    x=(p.x()-gap)*real_length/canvas_length;
    y=(canvas_width-p.y()+gap)*real_width/canvas_width;
    return QPoint(x,y);
}

void MainWindow::updateMapSize()
{
    if(ui->editMapX->text().isEmpty() || ui->editMapY->text().isEmpty())
    {
        QString dlgTitle="错误";
        QString strInfo="请先填写地图信息!";
        QMessageBox::critical(this,dlgTitle,strInfo);
        return;
    }
    int x=ui->editMapX->text().toInt();
    int y=ui->editMapY->text().toInt();
    real_length=x;
    real_width=y;
    ui->xlabel->setText(QString::number(x));
    ui->ylabel->setText(QString::number(y));
    processor->setMapSize(real_length,real_width);
}

void MainWindow::updateGraph()
{
    real_line.append(processor->line_data);
    update();
}
