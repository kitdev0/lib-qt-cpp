#ifndef HM_GPIO_H
#define HM_GPIO_H

#include <QObject>
#include <QIODevice>

#include "../../../../lib-qt-cpp/sw_module/sm_debug.h"

#ifndef _DEBUG_SAY_ONLY
#define _DEBUG_SAY_ONLY 0
#endif //_DEBUG_SAY_ONLY

#ifndef _DEBUG_WRITE_ONLY //Set write to se card
#define _DEBUG_WRITE_ONLY 1
#endif //_DEBUG_WRITE_ONLY

#ifndef _DEBUG_SAY_AND_WRITE
#define _DEBUG_SAY_AND_WRITE 2
#endif //_DEBUG_PRINT_AND_WRITE

//#define _HM_GPIO_DEBUG _DEBUG_SAY_ONLY
#define _HM_GPIO_DEBUG _DEBUG_WRITE_ONLY
//#define _HM_GPIO_DEBUG _DEBUG_SAY_AND_WRITE

#ifndef _OUTPUT
#define _OUTPUT true
#endif

#ifndef _INPUT
#define _INPUT  false
#endif

#ifndef _HIGH
#define _HIGH   true
#endif

#ifndef _LOW
#define _LOW    false
#endif

class HM_GPIO : public QObject
{
    Q_OBJECT
public:
    explicit HM_GPIO(QObject *parent = 0);
    ~HM_GPIO();
    void pinExport(uint16_t _pin);
    void pinUnExport(uint16_t _pin);
    void pinMode(uint16_t _pin, bool _mode);
    void digitalWrite(uint16_t _pin, bool _value);
    char digitalRead(uint16_t _pin);

private:
#ifdef _HM_GPIO_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _UC20_DEBUG
    void debug(QString data);
    bool writeByte(QString name,char value);
    bool writeStr(QString name,QIODevice::OpenMode mode,char *value);
    char readByte(QString name);

signals:

public slots:
};

#endif // HM_GPIO_H
