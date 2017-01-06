#include "hm_gpio_v101.h"
#include <QFile>

HM_GPIO::HM_GPIO(QObject *parent) : QObject(parent)
{
#ifdef _HM_GPIO_DEBUG
    logDebug = new SM_DEBUGCLASS("GPIO");
#endif
}

HM_GPIO::~HM_GPIO()
{
#ifdef _HM_GPIO_DEBUG
    delete logDebug;
#endif
}

//public
void HM_GPIO::pinExport(uint16_t _pin)
{
    QString command;
    command = String("echo %1 > /sys/class/gpio/export").arg(_pin);
    system(command.toStdString().c_str());
}

void HM_GPIO::pinUnExport(uint16_t _pin)
{
    QString command;
    command = String("echo %1 > /sys/class/gpio/unexport").arg(_pin);
    system(command.toStdString().c_str());
}

void HM_GPIO::pinMode(uint16_t _pin, bool _mode)
{
    if(_mode == _OUTPUT){
        QString command;
        command = "echo out > " + QString("/sys/class/gpio/gpio%1/direction").arg(_pin);
        system(command.toStdString().c_str());
    }
    else if(_mode == _INPUT){
        QString command;
        command = "echo in > " + QString("/sys/class/gpio/gpio%1/direction").arg(_pin);
        system(command.toStdString().c_str());
    }
}

void HM_GPIO::digitalWrite(uint16_t _pin, bool _value)
{
    if(_value == _HIGH){
        QString command;
        command = "echo 1 > " + QString("/sys/class/gpio/gpio%1/value").arg(_pin);
        system(command.toStdString().c_str());
    }
    else if(_value == _LOW){
        QString command;
        command = "echo 0 > " + QString("/sys/class/gpio/gpio%1/value").arg(_pin);
        system(command.toStdString().c_str());
    }
}

char HM_GPIO::digitalRead(uint16_t _port)
{
    return readByte(QString("/sys/class/gpio/gpio%1/value").arg(_port));
}


//private:

void HM_GPIO::debug(String data)
{
#ifdef _HM_GPIO_DEBUG
#if _HM_GPIO_DEBUG == _DEBUG_SAY_ONLY
    logDebug->say(data);
#elif _HM_GPIO_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _HM_GPIO_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}

char HM_GPIO::readByte(QString name)
{
    char data;
    QFile fp(name);

    if (!fp.open(QIODevice::ReadOnly)) return ' ';
    int nbyte = fp.read(&data,1);
    fp.close();

    return (nbyte ? data : ' ');
}
