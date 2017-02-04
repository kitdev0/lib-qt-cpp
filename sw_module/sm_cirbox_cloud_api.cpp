#include "sm_cirbox_cloud_api.h"

SM_CIRBOX_CLOUD_API::SM_CIRBOX_CLOUD_API(SM_GSM_MODULE *my_ethernet, QObject *parent) :
    QObject(parent)
{
    logDebug = new SM_DEBUGCLASS("Cloud-API");
    ethernet = my_ethernet;
    check_api_buff_to_send_timer = new QTimer;
    client_ping_pulling_timer = new QTimer;
    systime_timer = new QTimer;

    json_api_data = new QJsonObject;


    connect(this, SIGNAL(signalConnectServerOK()),this,SLOT(slotStartToCheckAPIBuff()));
    connect(check_api_buff_to_send_timer, SIGNAL(timeout()),this,SLOT(slotCheckAPIBuffToSend()));
    connect(client_ping_pulling_timer,SIGNAL(timeout()),this,SLOT(slotClientPing()));
    connect(ethernet,SIGNAL(signalInternetIsOK()),this,SLOT(slotSyncTime()));
}

SM_CIRBOX_CLOUD_API::~SM_CIRBOX_CLOUD_API()
{
    delete logDebug;
    delete ethernet;
    delete json_api_data;
    delete check_api_buff_to_send_timer;
}

void SM_CIRBOX_CLOUD_API::debug(String data)
{
#ifdef _CLOUD_API_DEBUG
#if _CLOUD_API_DEBUG == _DEBUG_SAY_ONLY
    logDebug->sayln(data);
#elif _CLOUD_API_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _CLOUD_API_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}

uint16_t SM_CIRBOX_CLOUD_API::responseStatus(QJsonDocument *_json_response)
{
    uint16_t _status = _json_response->object().value(String("status")).toInt();
//    debug("response status >> " + String::number(_status,10));
    return _status;
}

String SM_CIRBOX_CLOUD_API::responseMessage(QJsonDocument *_json_response)
{
    String _message = _json_response->object().value(String("message")).toString();
//    debug("response message >> " + _message);
    return _message;
}

uint8_t SM_CIRBOX_CLOUD_API::responseCmd(QJsonDocument *_json_response)
{
    uint8_t _cmd = _json_response->object().value(String("cmd")).toInt();
    //debug("response cmd >> " + String::number(_cmd,10));
    return _cmd;
}

bool SM_CIRBOX_CLOUD_API::reportData(QJsonDocument *_json_report)
{
    clientPingResetTimer();

//    debug("## Report transaction ##");
    String _table_no = _json_report->object().value(String("table_no")).toString();
    if(_table_no.toInt() > _TABLE_NO_MAX || _table_no.toInt() < 0){
        debug("Warning >> table-no is incorrect !!");
        return 0;
    }

    String _sub_url = "";

    if(_table_no.size() < 2)
        _sub_url = "table0" + _table_no;
    else
        _sub_url = "table" + _table_no;

    String _url = String(_API_V1_URL) + _sub_url;
    String _api = "";

    QJsonObject _json_object = _json_report->object();
//    _json_object.insert("m_time",getMachineTime());
    _json_object.remove("table_no");
    _json_object.insert("serialno",serialno);

    _api = QJsonDocument(_json_object).toJson(QJsonDocument::Compact);
    debug("Report data >> " + _api);

    if(!ethernet->http->setURL(_url)){
        //debug("#A1");
        ethernet->internet->resetConnecting();
        return 0;
    }
    if(ethernet->http->postMethod(_api) != _HTTP_STATUS_OK){
        //debug("#A2");
        ethernet->internet->resetConnecting();
        emit signalSetLEDServer(_LED_OFF);
        return 0;
    }

    flag_wait_to_read_response = true;
    QTimer::singleShot(_READ_METHOD_DELAY_TIME,this,SLOT(slotReadResponseAPI()));
    return 1;
}

String SM_CIRBOX_CLOUD_API::getMachineTime(void)
{
    String _time = logDebug->currentDay() + " " + logDebug->currentTime();
    return _time;
}

void SM_CIRBOX_CLOUD_API::checkToExecuteCmd(uint8_t _cmd)
{
    switch (_cmd)
    {
    case 1://shutdown
        debug("!! Shutdown !!");
        emit signalOffLEDAll();
        flag_set_led_off_all = true;
        QTimer::singleShot(5000,this,SLOT(slotCloudBoxShutdown()));
        break;
    case 2://reboot
        debug("!! Reboot !!");
        emit signalOffLEDAll();
        flag_set_led_off_all = true;
        QTimer::singleShot(5000,this,SLOT(slotCloudBoxReboot()));
        break;
    default:
        debug("!! CMD Not found !!");
        break;
    }
}

//private

//private slot
void SM_CIRBOX_CLOUD_API::slotCloudBoxShutdown(void)
{
    String command = "sudo shutdown now";
    system(command.toStdString().c_str());
}

void SM_CIRBOX_CLOUD_API::slotCloudBoxReboot(void)
{
    String command = "sudo reboot";
    system(command.toStdString().c_str());
}

void SM_CIRBOX_CLOUD_API::slotStartToCheckAPIBuff(void)
{
    check_api_buff_to_send_timer->start(_CHECK_API_BUFF_TO_SEND_TIMER);
}

void SM_CIRBOX_CLOUD_API::slotCheckAPIBuffToSend(void)
{
    if(!cloud_box_ready){
        return;
    }
    if(ethernet->moduleCannotUse()){
        ethernet->slotResetGsmModule();
        return;
    }

    if(!apiDataToSendAvailable()){
        check_api_buff_to_send_timer->stop();
        return;
    }
    else
        slotStartToCheckAPIBuff();

    if(flag_wait_to_read_response){
        return;
    }

    if(apiDataToSendAvailable())
    {
        if(!ethernet->internet->isConnect()){
            if(!ethernet->internet->connect()){
                //debug("Can't connect internet!!");
            }
            return;
        }

        if(!json_api_buff[last_api_buff].isEmpty()){
            QJsonDocument _json_doc(json_api_buff[last_api_buff]);
            reportData(&_json_doc);
        }
        else{
            last_api_buff = (uint16_t)(last_api_buff + 1) % _API_BUFFER_SIZE;
            debug("data >> empty!!");
        }
    }

}

void SM_CIRBOX_CLOUD_API::slotReadResponseAPI(void)
{
    String _api = ethernet->http->readMethod(true);

    if(_api.size() > 0)
    {
//        debug("Response API : " + _api);
        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());
        uint16_t _response_status = responseStatus(&_json_response);
        String _response_message = responseMessage(&_json_response);
        uint8_t _response_cmd = responseCmd(&_json_response);

        if(_response_status < _HTTP_STATUS_INTERNAL_SERVER_ERROR)
        {
            last_api_buff = (uint16_t)(last_api_buff + 1) % _API_BUFFER_SIZE;
        }

        if(_response_status == _HTTP_STATUS_OK || _response_message == _MESSAGE_SUCCESS){
            debug("Response Message >> " + _response_message);
            emit signalResponseAPISuccess();
        }
        else{
            debug("Response API : Unsuccess >> " + _api);
            emit signalResponseAPIUnsuccess();
        }

        if(_response_cmd > 0){
            debug("Response CMD >> " + String::number(_response_cmd,10));
            checkToExecuteCmd(_response_cmd);
        }

        emit signalSetLEDServer(_LED_ON);
    }
    else{
        debug("response data is empty!!");
        emit signalSetLEDServer(_LED_OFF);
    }
    flag_wait_to_read_response = false;
}

void SM_CIRBOX_CLOUD_API::slotReadResponseAPIClientPing()
{
    String _api = ethernet->http->readMethod(true);

    if(_api.size() > 0)
    {
//        debug("response api >> " + _api);
        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());
        uint16_t _response_status = responseStatus(&_json_response);
        String _response_message = responseMessage(&_json_response);
        uint8_t _response_cmd = responseCmd(&_json_response);

        if(_response_status != _HTTP_STATUS_OK || _response_message != _MESSAGE_SUCCESS){
            debug("Client Ping - Json Response error >> " + _api);
        }

        if(_response_cmd > 0){
            debug("Response Cmd >> " + String::number(_response_cmd,10));
            checkToExecuteCmd(_response_cmd);
        }

        if(!flag_set_led_off_all)
            emit signalSetLEDServer(_LED_ON);
    }
    else{
        debug("ClientPing >> Response data is empty!!");
        emit signalSetLEDServer(_LED_OFF);
    }
    flag_wait_to_read_response = false;
}

//---------- public ---------//
void SM_CIRBOX_CLOUD_API::setSerialNo(String _serial_no)
{
//    debug("set serial no.");
    if(_serial_no.size() == _SERIAL_NUMBER_SIZE_DEF){
        debug("serial-no >> " + _serial_no);
        serialno = _serial_no;
    }
    else{
        debug("serial-no >> incorrect!!");
    }
}

bool SM_CIRBOX_CLOUD_API::syncTime(String _gmt)
{
    clientPingResetTimer();

    debug("## getTimeZone ##");

    String _url = "http://cirbox.cloud/api/v1/synctime?serialno=" + serialno + "&gmt=" + _gmt;
    String _api = "";

    if(!ethernet->http->setURL(_url)){
        debug("getTimeZone >> Unsuccess !!");
        ethernet->internet->resetConnecting();
        return 0;
    }
    if(ethernet->http->getMethod(true) < 0){
        debug("getTimeZone >> Unsuccess !!");
        ethernet->internet->resetConnecting();
        emit signalSetLEDServer(_LED_OFF);
        return 0;
    }

    SM_DELAY::delay_ms(2000);

    _api = ethernet->http->readMethod(true);

    if(_api.size() > 0)
    {
//        debug("response api >> " + _api);

        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());

        if(responseStatus(&_json_response) != _HTTP_STATUS_OK || responseMessage(&_json_response) != _MESSAGE_SUCCESS){
            debug(_json_response.toJson(QJsonDocument::Compact));
            debug("Sync Time - Json Response error");
            return 0;
        }
        else{
            QJsonObject _json_value = _json_response.object().value("result").toObject();
            String _day = _json_value.value("day").toString();
            String _month = _json_value.value("mounth").toString();
            String _year = _json_value.value("year").toString();
            String _hour = _json_value.value("hour").toString();
            String _minute = _json_value.value("minute").toString();
            String _second = _json_value.value("second").toString();
//            debug("day >> " + _day);
//            debug("month >> " + _month);
//            debug("year >> " + _year);
//            debug("hour >> " + _hour);
//            debug("minute >> " + _minute);
//            debug("second >> " + _second);
            String _date = _year + _month + _day;
            String _time = _hour + ":" + _minute + ":" + _second;
            logDebug->setDateTime(_date, _time);
            debug("getTimeZone >> Success");
            debug("Sync Time >> " + _date + " " + _time);
            last_time_syntime = QDateTime::currentDateTime().toString("dd");
            cloud_box_ready = true;
            emit signalConnectServerOK();//Start to check api buffer
            emit signalSetLEDServer(_LED_ON);
            return 1;
        }
    }
    debug("getTimeZone >> Sync Time - API Response error");
    debug("getTimeZone >> Unsuccess !!");
    cloud_box_ready = false;
    return 0;
}

bool SM_CIRBOX_CLOUD_API::reqUpdateAPIData(void)
{
    if(!ethernet->moduleIsReady()){
        reqClrJsonAPIData();
        return 0;
    }
    if(!json_api_data->isEmpty())
    {
        json_api_data->insert("m_time",getMachineTime());
        json_api_buff[api_buff] = *json_api_data;
        api_buff = (uint16_t)(api_buff + 1) % _API_BUFFER_SIZE;
        reqClrJsonAPIData();
        slotCheckAPIBuffToSend();
        return 1;
    }
    else{
        debug("reqUpdateAPIData - Json data >> empty!!");
        return 0;
    }
}

bool SM_CIRBOX_CLOUD_API::reqClrJsonAPIData()
{
//    debug("request clear all transaction data.");
    delete json_api_data;
    json_api_data = new QJsonObject;
    if(json_api_data->isEmpty()){
//        debug("clear >> success.");
        return 1;
    }
    else{
        debug("clear >> unsuccess.");
        return 0;
    }
}

bool SM_CIRBOX_CLOUD_API::setAPIData(String _table_no, String _api_id, String _data)
{
//    debug("set data of tran-report");
    if(_table_no.size() && _api_id.size() && _data.size())
    {
        debug("# setAPIData");
        if(_api_id.indexOf("api") == -1){
            debug("api_id >> incorrect!!");
            return 0;
        }
        if(_table_no.toInt() < 0 || _table_no.toInt() > _TABLE_NO_MAX){
            debug("table_no >> incorrect!!");
            return 0;
        }
        if(_api_id.indexOf("api") != -1){
            QJsonDocument _json_doc;
            QJsonObject _json_object;
            _json_object.insert(_api_id,_data);
            _json_object.insert("table_no",_table_no);
            _json_doc.setObject(_json_object);
            if(_json_doc.isEmpty())
                return 0;
            else{
                json_api_data->insert(_api_id,_data);
                json_api_data->insert("table_no",_table_no);
                _json_doc.setObject(*json_api_data);
                debug("data >> " + _json_doc.toJson(QJsonDocument::Compact));
            }
            return 1;
        }
    }
    debug("parameter >> empty!!");
    return 0;
}

uint16_t SM_CIRBOX_CLOUD_API::apiDataToSendAvailable(void)
{
    return (uint16_t)(_API_BUFFER_SIZE + api_buff - last_api_buff) % _API_BUFFER_SIZE;
}

void SM_CIRBOX_CLOUD_API::clientPingResetTimer()
{
    client_ping_pulling_timer->start(_CLIENT_PING_PULLING_TIME);
}

void SM_CIRBOX_CLOUD_API::slotReqUpdateAPI(QJsonDocument *_json_report)
{
    QJsonObject _json_object = _json_report->object();

    if(!_json_object.isEmpty())
    {
        _json_object.insert("m_time",getMachineTime());
        json_api_buff[api_buff] = _json_object;
        api_buff = (uint16_t)(api_buff + 1) % _API_BUFFER_SIZE;
        slotCheckAPIBuffToSend();
    }
    else{
        debug("slotReqUpdateAPI - Json data >> empty!!");
    }
}

void SM_CIRBOX_CLOUD_API::slotClientPing(void)
{
    client_ping_pulling_timer->stop();

    if(ethernet->moduleCannotUse()){
        ethernet->slotResetGsmModule();
        clientPingResetTimer();
        return;
    }

    if(flag_wait_to_read_response){
        clientPingResetTimer();
        return;
    }

    if(!ethernet->internet->isConnect()){
        if(!ethernet->internet->connect()){
            //debug("Can't connect internet!!");
            clientPingResetTimer();
            return;
        }
    }

    String _url = "http://cirbox.cloud/api/v1/ping?serialno=" + serialno;
    String _api = "";

    if(!ethernet->http->setURL(_url)){
        ethernet->internet->resetConnecting();
        clientPingResetTimer();
        return;
    }
    if(ethernet->http->getMethod(true) < 0){
        ethernet->internet->resetConnecting();
        clientPingResetTimer();
        emit signalSetLEDServer(_LED_OFF);
        return;
    }

    flag_wait_to_read_response = true;
    QTimer::singleShot(_READ_METHOD_DELAY_TIME,this,SLOT(slotReadResponseAPIClientPing()));

    if(QDateTime::currentDateTime().toString("dd") != last_time_syntime){
        QTimer::singleShot(5000,this,SLOT(slotGetTimeFromServer()));
    }
//    else{
//        debug("#AB");
//    }

    clientPingResetTimer();
}

void SM_CIRBOX_CLOUD_API::slotSyncTime(void)
{
    if(!cloud_box_ready)
    {
        if(!ethernet->internet->isConnect())
            ethernet->internet->connect();

        if(!syncTime("+7")){
            QTimer::singleShot(2000,this,SLOT(slotSyncTime()));
        }
        else{
            ethernet->internet->disConnect();
        }
    }
}

void SM_CIRBOX_CLOUD_API::slotGetTimeFromServer(void)
{
    if(flag_wait_to_read_response){
        return;
    }

    if(!ethernet->internet->isConnect())
        ethernet->internet->connect();

    syncTime("+7");
}
