#ifndef SM_ZIP_UNZIP_FILE_H
#define SM_ZIP_UNZIP_FILE_H

#include <QObject>
#include <QFile>

class SM_ZIP_UNZIP_FILE : public QObject
{
    Q_OBJECT
public:
    explicit SM_ZIP_UNZIP_FILE(QObject *parent = 0);
    void zip(QString filename, QString zip_filename);
    void unZip(QString zip_filename, QString filename);
signals:

public slots:
};

#endif // SM_ZIP_UNZIP_FILE_H
