#ifndef QMQTT_MAIN_H
#define QMQTT_MAIN_H

#include "../../../../lib-qt-cpp/qmqtt/qmqtt.h"
#include "../../../../lib-qt-cpp/qmqtt/qmqtt_client.h"
#include <QString>
#include <QObject>
#include <QDebug>


using QMQTT::Client;
using QMQTT::Will;
using QMQTT::Message;

class qmqtt_main
{

public:
    qmqtt_main();

public slots:
    void mqttConnected();
    void mqttDisconnected();
    void mqttReceived(const QMQTT::Message &msg);

signals:

private slots:

private:
    QMQTT::Client* client;

};

#endif // QMQTT_MAIN_H
