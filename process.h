#ifndef PROCESS_H
#define PROCESS_H

#include <QThread>
#include "public.h"
#include <QDebug>

class process : public QThread
{
    Q_OBJECT
signals:
    void processed();

public:
    process();
    bool isbusy;                                //线程状态
    QVector<QPoint> line_data;                  //车辆轨迹数据
    void setCacheData(QByteArray d);
    void setReplyFlag(bool *flag);
    void processData();
    int getDataIndex(QByteArray data);
    void setMapSize(int xszie,int ysize);

private:
    QByteArray cachedata;                       //数据缓冲区
    QByteArray rec_data;                        //数据待处理区
    bool *reply_flag;                           //从mainwindows传入的回复标记位的指针
    int map_length,map_width;                   //地图长、宽
    QPoint changeToPainterPoint(QPoint p);

protected:
    void run();
};

#endif // PROCESS_H
