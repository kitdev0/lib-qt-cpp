#include "sm_debug.h"

SM_DEBUG::SM_DEBUG(QString _head, QObject *parent) :
    QObject(parent)
{
    log_path = _LOG_PATH;
    header = _head;
}

//  private:
QString SM_DEBUG::setFormat(QString _data)
{
    QString _str;
    _str = currentTime() + " " + header + " : " + _data;
    return _str;
}

QString SM_DEBUG::currentTime(void)
{
    return QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
}

QString SM_DEBUG::currentDay(void)
{
    return QDateTime::currentDateTime().toString("dd-MM-yyyy");
}

void SM_DEBUG::setDateTime(QString _date, QString _time)
{
//    sayln("set date-time");
//    sayln("date >> " + _date);
//    sayln("time >> " + _time);

//    QQString command = "sudo date " + _date + _time + _year;
//    system(command.toStdQString().c_str());

#ifdef Q_OS_OSX

#else
    QString command = "timedatectl set-ntp 0";
    system(command.toStdQString().c_str());

    command = "date +%Y%m%d -s " + _date;
    system(command.toStdQString().c_str());

    command = "date +%T -s " + _time;
    system(command.toStdQString().c_str());
#endif
}

void SM_DEBUG::delOldFile()
{
    checkOldDir();
}

bool SM_DEBUG::checkAndMakeLogDir(QString path)
{
    QDir dir(path);
    if(dir.exists()){
        return 1;
    }
    else{
        if(!dir.mkpath(".")){
            return 0;
        }
        else{
            return 1;
        }
    }
    return 0;
}

bool SM_DEBUG::checkAndWriteLogFile(QString path,QString data)
{
    QFile file(path);
    if(file.exists()){
        //qDebug() << "log file is exists";
        if(file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)){
            //qDebug() <<  "write append file";
            //QTextStream outStream(&file);
            QTextStream outStream(&file);
            if(data.indexOf("\r\n") != -1)
                outStream << data;
            else
                outStream << data + "\r\n";
            file.close();
            return 1;
        }
        else
        {
            qDebug() << "log file cannot open";
            file.close();
        }
    }
    else{
        if(file.open(QIODevice::WriteOnly)){
            //qDebug() <<  "write new file";
            QTextStream outStream(&file);
            if(data.indexOf("\r\n") != -1)
                outStream << data;
            else
                outStream << data + "\r\n";
            file.close();
            return 1;
        }
        else{
            qDebug() << "log file : " << path << " --> Cannot creat.";
            file.close();
        }
    }
    return 0;
}

QString SM_DEBUG::nextFileName(QString path)
{
    QDir dir(path);

    QStringList files;
    files << "*.log";

    dir.setNameFilters(files);
    QStringList files2 = dir.entryList();

    QString fileName = "001";

    if(files2.size() >= 1)
    {
        for( int i=0;i < files2.size();i++){
            QStringRef _str = files2.at(i).leftRef(3);
            if(_str.toInt() > fileName.toInt()){
                fileName = _str.toString();
            }
            //qDebug() << "file : " << _str;
        }
        QFileInfo fileInfo(path+"/"+fileName+".log");

        if(fileInfo.size() >= _FILE_SIZE)
        {
            int i = fileName.toInt()+1;
            fileName = QString("%1").arg(i,3,10,QChar('0'));
        }
        //qDebug() << "fileName = " << fileName;
        return fileName;
    }
    else
    {
        //qDebug() << "fileName = " << fileName;
        return fileName;
    }
}

void SM_DEBUG::checkOldDir(void)
{
    QDir dir(log_path);

    dir.setFilter(QDir::Dirs);

    QStringList dirList = dir.entryList();
    for( int i=0;i < dirList.size();i++)
    {
        if(dirList.at(i).size() > 2)
        {
            QFileInfo fileInfo(log_path+"/"+dirList.at(i));
            //qDebug() << fileInfo.lastModified().toQString("dd_MM_yyyy");
            if(fileInfo.lastModified().daysTo(QDateTime::currentDateTime()) >= _DAY_TO_DEL_DIR){
                QDir oldDir(log_path+"/"+dirList.at(i));
                qDebug() << "delete dir : " << fileInfo.absoluteFilePath();
                oldDir.removeRecursively();
            }

            checkOldFile(fileInfo);
        }
    }
}

void SM_DEBUG::checkOldFile(QFileInfo fileInfo)
{
    QStringList files;
    files << "*.log";

    QDir _dir(fileInfo.absoluteFilePath());
    _dir.setNameFilters(files);
    QStringList files2 = _dir.entryList();
    QString fileName = "001";
    QStringRef _str;
    if(files2.size() > _FILE_NO_MAX){
        int no2Del = (int)files2.size() - _FILE_NO_MAX;
        //qDebug() << "file no. = " << QQString("%1").arg(files2.size(),0,10);
        for( int i=0;i < files2.size();i++){
            _str = files2.at(i).leftRef(3);
            if(_str.toInt() > fileName.toInt()){
                fileName = _str.toString();
            }
        }
        //qDebug() << "last file : " << fileName;
        int p = (fileName.toInt() - _FILE_NO_MAX);
        int delNo = 0;
        for( int i=0;i < files2.size();i++){
            _str = files2.at(i).leftRef(3);
            if(_str.toInt() <= p){
                //qDebug() << "delete file = " << files2.at(i);
                qDebug() << "delete file path = " << fileInfo.absoluteFilePath()+"/"+files2.at(i);
                QFile _file(fileInfo.absoluteFilePath()+"/"+files2.at(i));
                _file.remove();
                delNo++;
                if(delNo >= no2Del){
                    qDebug() << "delete completed";
                    break;
                }
            }
        }
    }
}

//  public:

void SM_DEBUG::setHeader(QString _header)
{
    header = _header;
}

void SM_DEBUG::setLogPath(QString _path)
{
    QString _str = "Set path log file to : ";
    _str += _path;
    sayln(_str);

    log_path = _path;
}

void SM_DEBUG::say(QString _data)
{
    QString _str = setFormat(_data);
    qDebug() << _str;
    emit logEvent(_str);
}

void SM_DEBUG::sayln(QString _data)
{
    if(_data.indexOf("\r\n") == -1)
        _data += "\r\n";
    say(_data);
}

void SM_DEBUG::sayAndWriteLog(QString _data)
{
    writeLog(_data);
    sayln(_data);
}

void SM_DEBUG::writeLog(QString _data)
{
    if(STATIC_BOOL_LOG_CAN_WRITE)
    {
        QString _logPath = log_path + QDateTime::currentDateTime().toString("dd_MM_yyyy");
        QString _str = setFormat(_data);
        if(!checkAndMakeLogDir(_logPath))
        {
            sayln("Cannot make Log Dir.");
            STATIC_BOOL_LOG_CAN_WRITE = false;
        }
        else{
            _logPath += "/" + nextFileName(_logPath) + ".log";
            if(!checkAndWriteLogFile(_logPath,_str)){
                sayln("Cannot write log file.");
                STATIC_BOOL_LOG_CAN_WRITE = false;
            }
        }
    }
}

