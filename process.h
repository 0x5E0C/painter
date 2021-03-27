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
    bool isbusy;
    QVector<QPoint> line_data;
    void setCacheData(QByteArray d);
    void setReplyFlag(bool *flag);
    void processData();
    int getDataIndex(QByteArray data);
    void setMapSize(int xszie,int ysize);

private:
    QByteArray cachedata;
    QByteArray rec_data;
    bool *reply_flag;
    int map_length,map_width;
    QPoint changeToPainterPoint(QPoint p);

protected:
    void run();
};

#endif // PROCESS_H
