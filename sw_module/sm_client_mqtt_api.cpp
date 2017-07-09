#include "sm_client_mqtt_api.h"

SM_CLIENT_MQTT_API::SM_CLIENT_MQTT_API(QObject *parent) : QObject(parent)
{
    logDebug = new SM_DEBUGCLASS("MQTT-API");
    mqttClient = new QMQTT::Client();
    client_online_time = new QTimer;
    try_to_connect_broker_timer = new QTimer;

    connect(client_online_time,SIGNAL(timeout()),this,SLOT(slotCheckClientTimeout()));
    connect(this,SIGNAL(signalCheckReportData(QJsonDocument*)),this,SLOT(slotCheckReportData(QJsonDocument*)));
    connect(this,SIGNAL(signalCheckStatusData(QJsonDocument*)),this,SLOT(slotCheckStatusData(QJsonDocument*)));
    connect(mqttClient, SIGNAL(connected()), this, SLOT(slotMqttConnected()));
    connect(mqttClient, SIGNAL(disconnected()), this, SLOT(slotMqttDisconnected()));
    connect(mqttClient, SIGNAL(received(const QMQTT::Message &)), this, SLOT(slotMqttReceived(const QMQTT::Message &)));
    connect(try_to_connect_broker_timer,SIGNAL(timeout()),this,SLOT(slotTryToConnectBroker()));

}

SM_CLIENT_MQTT_API::~SM_CLIENT_MQTT_API()
{
    delete logDebug;
    delete mqttClient;
}

void SM_CLIENT_MQTT_API::init()
{
    slotInit();
}

void SM_CLIENT_MQTT_API::setClientIdList(QJsonDocument *_json_doc)
{
    QJsonObject _json_data = _json_doc->object();
    client_no = _json_data.value("client_no").toInt();
    debug("Client NO = " + String::number(client_no));
    for(int i=0; i < client_no; i++)
    {
        machine_status[i].id = _json_data.value(String::number(i+1)).toString();
    }
}

void SM_CLIENT_MQTT_API::initMachineStatus(void)
{
    for(int i=0; i < client_no; i++)
    {
        machine_status[i].c_state = false;
        machine_status[i].c_status = "";
        machine_status[i].m_status = "NULL";
        machine_status[i].e_code = "NULL";
    }
}

void SM_CLIENT_MQTT_API::slotMqttConnected(void)
{
    debug("Connect to Broker >> OK");
    mqttClient->subscribe("report",0);
    mqttClient->subscribe("status",0);
    client_online_time->start(_CLIENT_TIMEOUT_TIME);
//    if(try_to_connect_broker_timer->isActive())
//        try_to_connect_broker_timer->stop();
}

void SM_CLIENT_MQTT_API::slotMqttDisconnected(void)
{
    debug("Connect to Broker >> Fail");
    slotTryToConnectBroker();
}

void SM_CLIENT_MQTT_API::slotMqttReceived(const QMQTT::Message & _message)
{
    if(SM_CIRBOX_CLOUD_API::STATIC_BOOL_WAIT_TO_REBOOT){
        return;
    }
    String _topic = _message.topic();
    String _payload = String(_message.payload());

    if(!_payload.length()){
        debug("!! MqttReceive >> Empty Payload !!");
        return;
    }

    QJsonDocument _json_doc = QJsonDocument::fromJson(_payload.toUtf8());
    QJsonObject _json_api = _json_doc.object().value("api").toObject();

    if(_topic.indexOf("report") != -1){
        String _id = _json_api.value("1").toString();
        int16_t _mid = _json_doc.object().value("mid").toInt();

//        debug("# Get Report-Topic : Payload = " + _payload);
//        debug("# mid = " + String::number(_mid));

        if(!_id.length()){
            debug("!! MqttReceive >> ID Not found !!");
            return;
        }
//        debug("topic >> " + _topic);
        if(_mid != last_mid){
            if(_mid != -1){
                if(flag_reset_mahine){
                    flag_reset_mahine = false;
                    returnMessage(_id,_MESSAGE_RESET);
                }
                else
                    returnMessage(_id,_MESSAGE_SUCCESS);
                emit signalCheckReportData(&_json_doc);
            }
            last_mid = _mid;
        }
    }
    else if(_topic.indexOf("status") != -1){
        String _id = _json_doc.object().value("id").toString();
        if(flag_reset_mahine){
            flag_reset_mahine = false;
            returnMessage(_id,_MESSAGE_RESET);
        }
//        debug("topic >> " + _topic);
///        debug("payload >> " + _payload);
//        debug("ID-"+_id + "=" +_json_doc.object().value("status").toString());
//        returnMessage(_id,_MESSAGE_SUCCESS);
//        debug("#1 >> id = " + _id);
        emit signalCheckStatusData(&_json_doc);
    }
}

void SM_CLIENT_MQTT_API::slotCheckReportData(QJsonDocument *_json_doc)
{
    QJsonDocument _json_report;
    QJsonObject _json_object;

    QJsonObject _json_table = _json_doc->object();
    QJsonObject _json_api = _json_doc->object().value("api").toObject();

    String _table = _json_table.value("table").toString();

    _json_object.insert("table_no",_table);

    if(!_table.length()){
        debug("!! MqttReceive >> Table Not found !!");
        return;
    }

//    debug("table = " + _table);

    for(int i=0;i < _API_MAX;i++){
        String _value = _json_api.value(String::number(i)).toString();
        if(_value.length() > 0){
//            String _str = "api " + String::number(i) + " = " + _value;
            if(i < 10)
                _json_object.insert("api0"+String::number(i),_value);
            else
                _json_object.insert("api"+String::number(i),_value);
//            debug(_str);
        }
    }

    _json_report.setObject(_json_object);

    emit signalReportDataToCloud(&_json_report);

}

void SM_CLIENT_MQTT_API::slotCheckStatusData(QJsonDocument *_json_doc)
{
    QJsonObject _json_data = _json_doc->object();
    String _id = _json_data.value("id").toString();
    String _status = _json_data.value("status").toString();
    String _error_code = _json_data.value("e-code").toString();

    bool _flag_update = false;
    int _array = -1;

    String _str = "update status >> ";

//    _str += "ID : " + _id;
//    debug(_str);

//    _str = "update status >> ";
//    _str += "Status : " + _status;
//    debug(_str);
//    debug("#2");

    if(_id.size() > 0 && _status.size() > 0)
    {
        for(int i=0; i < client_no; i++){
            if(machine_status[i].id == _id){
//                debug("#3");
                _array = i;
                break;
            }
        }

        if(_array == -1){
            debug("client id >> Not found !!");
            return;
        }

//        debug("#4");

        machine_status[_array].c_state = true;
        machine_status[_array].c_status = "ONLINE";

        if(_status != machine_status[_array].m_status){
           machine_status[_array].m_status = _status;
           _flag_update = true;
        }

        if(_error_code.size() > 0){
            if(_error_code != machine_status[_array].e_code){
                machine_status[_array].e_code = _error_code;
                _str = "Update status >> ";
                _str += "ID:" + _id;
                _str += "-Error:" + _error_code;
                debug(_str);
                _flag_update = true;
            }
        }
        else{
            if(machine_status[_array].e_code != "-"){
                machine_status[_array].e_code = "-";
                _flag_update = true;
            }
        }
//        debug("#5");
        emit signalSetLEDClient(_LED_ON);
    }
    else
        debug("Status data >> Empty !!");

    if(_flag_update)
    {
        QJsonDocument _json_report;
        QJsonObject _json_object;

        _json_object.insert("table_no","1");
        _json_object.insert("api01",machine_status[_array].c_status);
        _json_object.insert("api02",_id);
        _json_object.insert("api03",machine_status[_array].m_status);
        _json_object.insert("api04",machine_status[_array].e_code);

        _json_report.setObject(_json_object);

        emit signalReportDataToCloud(&_json_report);
    }

}

void SM_CLIENT_MQTT_API::slotCheckClientTimeout(void)
{
    for(int i=0; i < client_no; i++)
    {
        if(machine_status[i].c_state == false && machine_status[i].c_status != "OFFLINE")
        {
            QJsonDocument _json_report;
            QJsonObject _json_object;

            machine_status[i].c_status = "OFFLINE";
            machine_status[i].m_status = "NULL";
            machine_status[i].e_code = "NULL";

            _json_object.insert("table_no","1");
            _json_object.insert("api01",machine_status[i].c_status);
            _json_object.insert("api02",machine_status[i].id);
            _json_object.insert("api03",machine_status[i].m_status);
            _json_object.insert("api04",machine_status[i].e_code);

            _json_report.setObject(_json_object);

            debug("client id : " + machine_status[i].id + " >> OFFLINE");

            emit signalReportDataToCloud(&_json_report);
            emit signalSetLEDClient(_LED_OFF);
        }
        machine_status[i].c_state = false;
    }
}

void SM_CLIENT_MQTT_API::slotTryToConnectBroker()
{
    if(try_to_connect_broker_timer->isActive())
        try_to_connect_broker_timer->stop();

    if(!mqttClient->isConnected()){
        debug("slotTryToConnectBroker");
        slotStopBroker();

        QTimer::singleShot(2500,this,SLOT(slotStartBroker()));
        QTimer::singleShot(5000,this,SLOT(slotInit()));
        QTimer::singleShot(8000,this,SLOT(slotStartTryToConnectBroker()));
    }
//    slotStartTryToConnectBroker();
}

void SM_CLIENT_MQTT_API::slotStopBroker()
{
    String _command = "service mosquitto stop";
    system(_command.toStdString().c_str());
}

void SM_CLIENT_MQTT_API::slotStartBroker()
{
    String _command = "service mosquitto start";
    system(_command.toStdString().c_str());
}

void SM_CLIENT_MQTT_API::slotInit()
{
    mqttClient->setClientId(_MQTT_ID);
    mqttClient->setUsername(_MQTT_NAME);
    mqttClient->setPassword(_MQTT_PASS);
    mqttClient->connect();
}

void SM_CLIENT_MQTT_API::slotStartTryToConnectBroker()
{
    try_to_connect_broker_timer->start(_TRY_TO_CONNECT_BROKER);
}

void SM_CLIENT_MQTT_API::slotResetMachine()
{
    debug("Reset Machine");
    flag_reset_mahine = true;
}

//private
void SM_CLIENT_MQTT_API::debug(String data)
{
#ifdef _CLIENT_MQTT_API_DEBUG
#if _CLIENT_MQTT_API_DEBUG == _DEBUG_SAY_ONLY
    logDebug->sayln(data);
#elif _CLIENT_MQTT_API_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _CLIENT_MQTT_API_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}

void SM_CLIENT_MQTT_API::returnMessage(String _id, String _message)
{
    QJsonDocument _json_doc;
    QJsonObject _json_object;
    QByteArray _data = "";
    QMQTT::Message my_message;

    _json_object.insert("id",_id);
    _json_object.insert("message",_message);

    _json_doc.setObject(_json_object);

    _data = _json_doc.toJson(QJsonDocument::Compact);
    debug("return data >> " + _data);

    my_message.setId(999);
    my_message.setTopic("return");
    my_message.setQos(0);
    my_message.setPayload(_data);
    mqttClient->publish(my_message);
}
