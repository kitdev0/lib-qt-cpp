#include "sm_client_mqtt_api_v101.h"

SM_CLIENT_MQTT_API::SM_CLIENT_MQTT_API(QObject *parent) : QObject(parent)
{
    logDebug = new SM_DEBUGCLASS("MQTT-API");
    mqttClient = new QMQTT::Client();
    client_online_time = new QTimer;

    connect(client_online_time,SIGNAL(timeout()),this,SLOT(slotCheckClientTimeout()));
    connect(this,SIGNAL(signalCheckReportData(QJsonDocument*)),this,SLOT(slotCheckReportData(QJsonDocument*)));
    connect(this,SIGNAL(signalCheckStatusData(QJsonDocument*)),this,SLOT(slotCheckStatusData(QJsonDocument*)));
    connect(mqttClient, SIGNAL(connected()), this, SLOT(slotMqttConnected()));
    connect(mqttClient, SIGNAL(disconnected()), this, SLOT(slotMqttDisconnected()));
    connect(mqttClient, SIGNAL(received(const QMQTT::Message &)), this, SLOT(slotMqttReceived(const QMQTT::Message &)));
}

SM_CLIENT_MQTT_API::~SM_CLIENT_MQTT_API()
{
    delete logDebug;
    delete mqttClient;
}

void SM_CLIENT_MQTT_API::init()
{
    mqttClient->setClientId(_MQTT_ID);
    mqttClient->setUsername(_MQTT_NAME);
    mqttClient->setPassword(_MQTT_PASS);
    mqttClient->connect();
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
}

void SM_CLIENT_MQTT_API::slotMqttDisconnected(void)
{
    debug("Connect to Broker >> Fail");
}

void SM_CLIENT_MQTT_API::slotMqttReceived(const QMQTT::Message & _message)
{
    String _topic = _message.topic();
    String _payload = String(_message.payload());

    if(!_payload.length())
        return;

    QJsonDocument _json_doc = QJsonDocument::fromJson(_payload.toUtf8());
    QJsonObject _json_api = _json_doc.object().value("api").toObject();

//    debug("topic >> " + _topic);
//    debug("payload >> " + _payload);

    if(_topic.indexOf("report") != -1){
        String _id = _json_api.value("1").toString();
        returnMessage(_id,_MESSAGE_SUCCESS);
        emit signalCheckReportData(&_json_doc);
    }
    else if(_topic.indexOf("status") != -1){
        String _id = _json_doc.object().value("id").toString();
//        returnMessage(_id,_MESSAGE_SUCCESS);
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

    if(!_table.length())
        return;

    debug("table = " + _table);

    for(int i=0;i < _API_MAX;i++){
        String _value = _json_api.value(String::number(i)).toString();
        if(_value.length() > 0){
            String _str = "api " + String::number(i) + " = " + _value;
            if(i < 10)
                _json_object.insert("api0"+String::number(i),_value);
            else
                _json_object.insert("api"+String::number(i),_value);
            debug(_str);
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

    if(_id.size() > 0 && _status.size() > 0)
    {
        for(int i=0; i < client_no; i++){
            if(machine_status[i].id == _id){
                _array = i;
                break;
            }
        }

        if(_array == -1){
            return;
        }

        machine_status[_array].c_state = true;
        machine_status[_array].c_status = "ONLINE";

        if(_status != machine_status[_array].m_status){
           machine_status[_array].m_status = _status;
           _flag_update = true;
        }

        if(_error_code.size() > 0){
            if(_error_code != machine_status[_array].e_code){
                machine_status[_array].e_code = _error_code;
                _flag_update = true;
            }
            _str = "update status >> ";
            _str += "Error : " + _id;
            debug(_str);
        }
        else{
            if(machine_status[_array].e_code != "-"){
                machine_status[_array].e_code = "-";
                _flag_update = true;
            }
        }
        emit signalSetLEDClient(_LED_ON);
    }
    else
        debug("Status data >> Empty !!");

    if(_flag_update)
    {
        QJsonDocument _json_report;
        QJsonObject _json_object;

        _json_object.insert("table_no","2");
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

            _json_object.insert("table_no","2");
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

//private
void SM_CLIENT_MQTT_API::debug(String data)
{
#ifdef _SM_CLIENT_MQTT_API_DEBUG
#if _SM_CLIENT_MQTT_API_DEBUG == _DEBUG_SAY_ONLY
    logDebug->sayln(data);
#elif _SM_CLIENT_MQTT_API_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _SM_CLIENT_MQTT_API_DEBUG == _DEBUG_SAY_AND_WRITE
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
