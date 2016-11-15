#ifndef SM_DELAY_H
#define SM_DELAY_H

#include <QThread>

class SM_DELAY
{
public:
    static void delay_us(unsigned long usecs){QThread::usleep(usecs);}
    static void delay_ms(unsigned long msecs){QThread::msleep(msecs);}
    static void delay_se(unsigned long secs){QThread::sleep(secs);}
};

#endif // SM_DELAY_H
