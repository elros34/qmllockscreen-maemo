#ifndef BQ2700_H
#define BQ2700_H

#include <QObject>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

class BQ2700 : public QObject
{
    Q_OBJECT
public:
    explicit BQ2700(QObject *parent = 0);
    
signals:
    
public slots:
    int getBatteryLevel();
    
};

#endif // BQ2700_H
