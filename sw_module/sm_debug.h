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
#define _FILE_SIZE 1000000 //1Mbyte
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

class SM_DEBUG : public QObject
{
    Q_OBJECT
public:
    explicit SM_DEBUG(QString _head,QObject *parent = 0);

    static bool STATIC_BOOL_LOG_CAN_WRITE;

    void setHeader(QString _header);
    void setLogPath(QString _data);
    void say(QString _data);
    void sayln(QString _data);
    void writeLog(QString _data);
    void sayAndWriteLog(QString _data);
    QString currentTime(void);
    QString currentDay(void);
    void setDateTime(QString _date, QString _time);
    void delOldFile(void);

private:
    QString log_path;
    QString header;
    QString setFormat(QString _data);
    bool checkAndMakeLogDir(QString path);
    bool checkAndWriteLogFile(QString path,QString data);
    QString nextFileName(QString path);
    void checkOldDir(void);
    void checkOldFile(QFileInfo fileInfo);

signals:
    void logEvent(QString _msg);
};

#endif // SM_DEBUG_H
