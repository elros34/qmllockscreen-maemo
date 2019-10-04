#include "bq2700.h"

//http://wiki.maemo.org/N900_Hardware_Bus_I2c
BQ2700::BQ2700(QObject *parent) :
    QObject(parent)
{
}

int BQ2700::getBatteryLevel()
{
    int i2cHandle;
    char rxBuffer[1];
    int batteryAdress = 0x55;
    int csoc = 0x2c;

    if((i2cHandle = open("/dev/i2c-2", O_RDWR)) < 0)
        return -1;

    //http://talk.maemo.org/showpost.php?p=1436015&postcount=23
    if(ioctl(i2cHandle, I2C_SLAVE_FORCE, batteryAdress) < 0)
        return -1;

    rxBuffer[0] = csoc;

    if(write(i2cHandle, rxBuffer, 1) < 0)
        return -1;

    if(read(i2cHandle, rxBuffer, 1) < 0)
        return -1;

    close(i2cHandle);

    return (int)rxBuffer[0];
}
