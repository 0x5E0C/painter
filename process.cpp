#include "process.h"

process::process()
{
    isbusy=false;
}

/*
函数名：setCacheData
参数：缓冲信息
返回值：无
功能：将接受到的信息存起用于拼接数据包消息
*/
void process::setCacheData(QByteArray d)
{
    cachedata.append(d);                                //将zigbee收到的数据加入待处理数据缓冲区
}

/*
函数名：setReplyFlag
参数：mainwindows.cpp中自动发送标志位的地址
返回值：无
功能：传入mainwindows.cpp中自动发送标志位用于后续线程同步
*/
void process::setReplyFlag(bool *flag)
{
    reply_flag=flag;                                    //回复标志位的指针
}

/*
函数名：getDataIndex
参数：需要处理的数据
返回值：包头位置
功能：用于寻找数据包头位置
*/
int process::getDataIndex(QByteArray data)
{
    int index=data.indexOf(0x5E);                       //找寻包头第一位0x5E
    if(index!=(data.size()-1))                          //找到的位置不在数据末
    {
        if(data.at(index+1)==0x0C)                      //查看0x5E下一位是否为0x0C
        {
            return index;
        }
    }
    return -1;
}

/*
函数名：setMapSize
参数：x轴长度、y轴长度
返回值：无
功能：设置地图大小
*/
void process::setMapSize(int xszie,int ysize)
{
    map_length=xszie;
    map_width=ysize;
}

/*
函数名：changeToPainterPoint
参数：实际场景的坐标点
返回值：画板内的坐标点
功能：将实际场景的坐标点转化为画板内的坐标点
*/
QPoint process::changeToPainterPoint(QPoint p)
{
    int x,y;
    x=p.x()*canvas_length/map_length+gap;
    y=canvas_width-p.y()*canvas_width/map_width+gap;
    return QPoint(x,y);
}

/*
函数名：run
参数：无
返回值：无
功能：process线程入口，用于拼接和解析数据包信息
*/
void process::run()
{
    quint16 sum;
    int index;
    isbusy=true;                                        //线程标记为正忙
    line_data.clear();
    while((cachedata.size()+rec_data.size())>=PACKET_LENGTH)
    {
        rec_data.append(cachedata);                     //将数据从缓冲区移至处理区
        cachedata.clear();                              //清除缓冲区
        index=getDataIndex(rec_data);
        if(index==-1)
        {
            rec_data.clear();                           //清除处理区数据
            continue;
        }
        QByteArray tempbuffer=rec_data.mid(index,PACKET_LENGTH);//截取数据到包头位置
        quint8 *rec_buffer=(quint8*)tempbuffer.data();  //转化数据格式
        rec_data=rec_data.mid(index+1);                 //将处理区的数据中的包头截断，避免下次重复检测
        sum=0;
        for(int i=0;i<8;i++)
        {
            sum+=rec_buffer[i];                         //计算检验和
        }
        if(sum!=(rec_buffer[8]<<8|rec_buffer[9]))
        {
            continue;
        }
        qint16 x=rec_buffer[4]<<8|rec_buffer[5];        //解析数据来源的车辆坐标
        qint16 y=rec_buffer[6]<<8|rec_buffer[7];
        if(rec_buffer[3]==REPLY_CMD)
        {
            *reply_flag=true;                           //回复标志位标记为true
        }
        else if(rec_buffer[3]==SENDPOS_CMD)
        {
            line_data.append(changeToPainterPoint(QPoint(x,y)));//添加数据到车辆轨迹点列表中
            emit processed();                           //发送处理完成信号
        }
    }
    isbusy=false;                                       //线程标记为空闲
}
