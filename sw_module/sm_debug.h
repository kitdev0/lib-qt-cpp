#ifndef SM_DEBUG_H
#define SM_DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QProcess>
#include "global.h"

#define _OSX_PLATFORM   0
#define _LINUX_PLATFORM 1
#define _RPI_PLATFORM   1

#ifndef _OS_PLATFORM
#define _OS_PLATFORM    _RPI_PLATFORM
#endif

#ifndef _LOG_PATH
#define _LOG_PATH "/User/kitdev/log/"
#endif

#ifndef _FILE_SIZE
//#define _FILE_SIZE 1000000 //1Mbyte
#define _FILE_SIZE 100000 //100Kbyte
#endif

#ifndef _DIR_SIZE
#define _DIR_SIZE 10000000 //10MByte
#endif

#ifndef _FILE_NO_MAX
#define _FILE_NO_MAX _DIR_SIZE/_FILE_SIZE
#endif


#ifndef _DAY_TO_DEL_DIR
#define _DAY_TO_DEL_DIR 7
#endif

typedef QString String;

class SM_DEBUGCLASS : public QObject
{
    Q_OBJECT
public:
    explicit SM_DEBUGCLASS(String _head,QObject *parent = 0);

    static bool STATIC_BOOL_LOG_CAN_WRITE;

    void setHeader(String _header);
    void setLogPath(String _data);
    void say(String _data);
    void sayln(String _data);
    void writeLog(String _data);
    void sayAndWriteLog(String _data);
    String currentTime(void);
    String currentDay(void);
    void setDateTime(String _date, String _time);
    void delOldFile(void);

private:
    String log_path;
    String header;
    String setFormat(String _data);
    bool checkAndMakeLogDir(String path);
    bool checkAndWriteLogFile(String path,String data);
    String nextFileName(String path);
    void checkOldDir(void);
    void checkOldFile(QFileInfo fileInfo);

signals:
    void logEvent(QString _msg);
};

#endif // SM_DEBUGCLASS_H
