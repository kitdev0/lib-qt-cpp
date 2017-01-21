#include <qmqtt/qmqtt_main.h>

qmqtt_main::qmqtt_main()
{
    client = new QMQTT::Client();

//    connect(client, SIGNAL(connected()), this,SLOT(mqttConnected()));
//    connect(client, SIGNAL(disconnected()), this, SLOT(mqttDisconnected()));
//    connect(client, SIGNAL(received(const QMQTT::Message &)), this, SLOT(mqttReceived(const QMQTT::Message &)));

}

void qmqtt_main::mqttConnected()
{
    qDebug()<<"connected";
//    client->subscribe(ui->txt_ReciveTopic->text(),0);
}

void qmqtt_main::mqttDisconnected()
{
    qDebug()<<"Disconnected";
}

void  qmqtt_main::mqttReceived(const QMQTT::Message &msg)
{
    QString message;
 //   if (msg.topic() == ui->txt_ReciveTopic->text())
//    {
        message = msg.payload()+"\r\n";
        //ui->txt_Recived->insertAction(;
//        ui->txt_Recived->insertPlainText(message);
//    }
        qDebug() << "Received";
}
