#include "sm_cirbox_cloud_api.h"

SM_CIRBOX_CLOUD_API::SM_CIRBOX_CLOUD_API(SM_GSM_MODULE *my_ethernet,QObject *parent) :
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
//    debug("response cmd >> " + String::number(_cmd,10));
    return _cmd;
}

QString SM_CIRBOX_CLOUD_API::responseDeviceID(QJsonDocument *_json_response)
{
    QString _id = _json_response->object().value(String("deviceid")).toString();
//    debug("response id >> " + _id);
    return _id;
}

QString SM_CIRBOX_CLOUD_API::responseCmdData(QJsonDocument *_json_response)
{
    String _cmd_data = _json_response->object().value(String("cmd_data")).toString();
//    debug("response cmd data >> " + _cmd_data);
    return _cmd_data;
}

bool SM_CIRBOX_CLOUD_API::reportData(QJsonDocument *_json_report)
{
    if(STATIC_BOOL_WAIT_TO_REBOOT){
        return false;
    }

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

    String _ccid = ethernet->sim->getCCID();
    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
        _ccid = "-";

    String _sq = String::number(ethernet->network->getSignalQuality(),10);
    if(_sq.length() == 0 || _sq.length() > 5)
        _sq = "-";

    _json_object.insert("ccid",_ccid);
    _json_object.insert("sq",_sq);

    _api = QJsonDocument(_json_object).toJson(QJsonDocument::Compact);
//    debug("Report data >> " + _api);

    if(!ethernet->http->setURL(_url)){
        //debug("#A1");
        debug("@10");
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
        emit signalSetLEDServer(_LED_OFF);
//        if(!ethernet->internet->resetConnecting()){
//            debug("@10");
//            cloud_box_ready = false;
////            QTimer::singleShot(5000,ethernet,SLOT(slotResetGsmModule()));
//            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//            emit signalSetLEDServer(_LED_OFF);
//        }
        return 0;
    }
    if(ethernet->http->postMethod(&_api) != _HTTP_STATUS_OK){
        debug("@11");
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
        emit signalSetLEDServer(_LED_OFF);

//        if(!ethernet->internet->resetConnecting()){
//            debug("@11");
//            cloud_box_ready = false;
//            QTimer::singleShot(5000,ethernet,SLOT(slotResetGsmModule()));
//            emit signalSetLEDServer(_LED_OFF);
//        }
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
        QTimer::singleShot(4000,this,SLOT(slotUnmountSDCard()));
        QTimer::singleShot(5000,this,SLOT(slotCloudBoxShutdown()));
        STATIC_BOOL_WAIT_TO_REBOOT = true;
        break;
    case 2://reboot
        debug("!! Reboot !!");
        emit signalOffLEDAll();
        flag_set_led_off_all = true;
        QTimer::singleShot(4000,this,SLOT(slotUnmountSDCard()));
        QTimer::singleShot(5000,this,SLOT(slotCloudBoxReboot()));
        STATIC_BOOL_WAIT_TO_REBOOT = true;
        break;
    case 3://get log
        if(date_get_log.size() > 0){
//            debug("!! Get Log !!");
            QTimer::singleShot(2000,this,SLOT(slotPostLog()));
        }
        else{
            debug("date log is empty!!");
        }
//        _cmd_data = "0";
        //
//        slotPostLog(_cmd_data);
//        emit signalOffLEDAll();
//        flag_set_led_off_all = true;
//        QTimer::singleShot(4000,this,SLOT(slotUnmountSDCard()));
//        QTimer::singleShot(5000,this,SLOT(slotCloudBoxReboot()));
//        STATIC_BOOL_WAIT_TO_REBOOT = true;
        break;
    default:
        debug("!! CMD Not found !!");
        break;
    }
}

void SM_CIRBOX_CLOUD_API::configSystem(QString _service_id)
{
    if(_service_id == _SERVICE_SC || _service_id == _SERVICE_MC)
    {
        cmdRemove_interfaceFile();

        if(_service_id == _SERVICE_SC){ //Single Client
//            debug("Set system to Single Client");

            slotCreatRunAppScript(_SERVICE_SC);

            String _command = "sudo update-rc.d mosquitto disable";
            system(_command.toStdString().c_str());

            _command = "sudo cp /Users/kitdev/CloudBoxApp/interfaces.sc /etc/network/interfaces";
            system(_command.toStdString().c_str());
        }

        if(_service_id == _SERVICE_MC){ //Multi Client
//            debug("Set system to Multi Client");

            slotCreatRunAppScript(_SERVICE_MC);

            String _command = "sudo update-rc.d mosquitto enable";
            system(_command.toStdString().c_str());

            _command = "sudo cp /Users/kitdev/CloudBoxApp/interfaces.mc /etc/network/interfaces";
            system(_command.toStdString().c_str());

            _command = "service mosquitto start";
            system(_command.toStdString().c_str());
        }

        setMachineType(_service_id);

//        emit signalOffLEDAll();
//        flag_set_led_off_all = true;
//        QTimer::singleShot(500,this,SLOT(slotUnmountSDCard()));
//        QTimer::singleShot(8000,this,SLOT(slotCloudBoxReboot()));
//        flag_wait_to_reboot = true;
    }
}

void SM_CIRBOX_CLOUD_API::cmdRemove_RunAppScript()
{
    String _command = "sudo rm /Users/kitdev/CloudBoxApp/runapp.sh";
    system(_command.toStdString().c_str());
}

void SM_CIRBOX_CLOUD_API::cmdChmodX_RunAppScript()
{
    String _command = "sudo chmod +x /Users/kitdev/CloudBoxApp/runapp.sh";
    system(_command.toStdString().c_str());
}

void SM_CIRBOX_CLOUD_API::cmdRemove_interfaceFile()
{
    String _command = "sudo rm /etc/network/interfaces";
    system(_command.toStdString().c_str());
}

//QString _deviceid = _json_value.value("deviceid").toString();
////            _deviceid = "04SC00001";
//if(_deviceid.indexOf(machine_type) == -1){ //don't right
//    debug("#A");
//    configSystem(_deviceid);
//    return false;
//}

void SM_CIRBOX_CLOUD_API::checkResponseIDToChangeService(QString _response_id)
{
    if(_response_id.size() > 0){
        QString _service_id = _response_id.mid(2,2);
//        debug("machine = " + machine_type);
        if(_service_id != machine_type){
            debug("Change service to " + _service_id);
            configSystem(_service_id);

            emit signalOffLEDAll();
            flag_set_led_off_all = true;
            QTimer::singleShot(8000,this,SLOT(slotUnmountSDCard()));
            QTimer::singleShot(10000,this,SLOT(slotCloudBoxReboot()));
            STATIC_BOOL_WAIT_TO_REBOOT = true;
        }
        else{
            STATIC_BOOL_WAIT_TO_REBOOT = false;
        }
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

void SM_CIRBOX_CLOUD_API::slotUnmountSDCard(void)
{
    String command = "sudo umount /dev/mmcblk0p1";
    system(command.toStdString().c_str());

    SM_DELAY::delay_ms(500);
    system(command.toStdString().c_str());

    SM_DELAY::delay_ms(500);
    system(command.toStdString().c_str());
}

void SM_CIRBOX_CLOUD_API::slotStartToCheckAPIBuff(void)
{
    check_api_buff_to_send_timer->start(_CHECK_API_BUFF_TO_SEND_TIMER);
}

void SM_CIRBOX_CLOUD_API::slotStopToCheckAPIBuff(void)
{
    check_api_buff_to_send_timer->stop();
}

void SM_CIRBOX_CLOUD_API::slotCheckAPIBuffToSend(void)
{
    if(!cloud_box_ready){
        return;
    }
    if(ethernet->moduleCannotUse()){
        debug("@8");
        cloud_box_ready = false;
//        ethernet->slotResetGsmModule();
        ethernet->slotResetNetwork();
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
                debug("# Can't connect internet!!");
                slotStopToCheckAPIBuff();
                cloud_box_ready = false;
                emit signalSetLEDServer(_LED_OFF);
                debug("@1");
                QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
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
        date_get_log = responseCmdData(&_json_response);
        QString _response_id = responseDeviceID(&_json_response);


        if(_response_status < _HTTP_STATUS_INTERNAL_SERVER_ERROR)
        {
            last_api_buff = (uint16_t)(last_api_buff + 1) % _API_BUFFER_SIZE;
        }

        if(_response_status == _HTTP_STATUS_OK || _response_message == _MESSAGE_SUCCESS){
//            debug("Response Message >> " + _response_message);
            emit signalResponseAPISuccess();
        }
        else{
            debug("Response API : Unsuccess >> " + _api);
            emit signalResponseAPIUnsuccess();
        }

        checkResponseIDToChangeService(_response_id);

        if(_response_cmd > 0){
//            debug("Response CMD >> " + String::number(_response_cmd,10));
            checkToExecuteCmd(_response_cmd);
        }

        if(!STATIC_BOOL_WAIT_TO_REBOOT)
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
        date_get_log = responseCmdData(&_json_response);
        QString _response_id = responseDeviceID(&_json_response);

        if(_response_status != _HTTP_STATUS_OK || _response_message != _MESSAGE_SUCCESS){
            debug("Client Ping - Json Response error >> " + _api);
        }

        checkResponseIDToChangeService(_response_id);

        if(_response_cmd > 0){
//            debug("Response Cmd >> " + String::number(_response_cmd,10));
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

//    debug("## getTimeZone ##");

    String _url = "";
    String _api = "";

    String _ccid = ethernet->sim->getCCID();
    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
        _ccid = "-";

    String _sq = String::number(ethernet->network->getSignalQuality(),10);
    if(_sq.length() == 0 || _sq.length() > 5)
        _sq = "-";

    _url = String(_CIRBOX_CLOUD_URL);
    _url += "/api/v1/synctime?";
    _url += "serialno=" + serialno;
    _url += "&gmt=" + _gmt;
    _url += "&ccid=" + _ccid;
    _url += "&sq=" + _sq;

//    debug("#URL = " + _url);

    if(!ethernet->http->setURL(_url)){
        debug("getTimeZone setURL >> Unsuccess !!");
        ethernet->internet->resetConnecting();
        cloud_box_ready = false;
        emit signalSetLEDServer(_LED_OFF);
        return 0;
    }
    if(ethernet->http->getMethod(true) < 0){
        debug("getTimeZone getMethod >> Unsuccess !!");
        ethernet->internet->resetConnecting();
        cloud_box_ready = false;
        emit signalSetLEDServer(_LED_OFF);
        return 0;
    }

    SM_DELAY::delay_ms(2000);

    _api = ethernet->http->readMethod(true);

    if(_api.size() > 0)
    {
//        debug("response api >> " + _api);

        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());
        uint8_t _response_cmd = responseCmd(&_json_response);
        date_get_log = responseCmdData(&_json_response);
        QString _response_id = responseDeviceID(&_json_response);

        checkResponseIDToChangeService(_response_id);

        if(_response_cmd > 0){
//            debug("Response Cmd >> " + String::number(_response_cmd,10));
            checkToExecuteCmd(_response_cmd);
        }

        if(responseStatus(&_json_response) != _HTTP_STATUS_OK || responseMessage(&_json_response) != _MESSAGE_SUCCESS){
            debug(_json_response.toJson(QJsonDocument::Compact));
            debug("Sync Time - Json Response error");
            return 0;
        }
        else{
            QJsonObject _json_value = _json_response.object().value("result").toObject();
            QString _day = _json_value.value("day").toString();
            QString _month = _json_value.value("mounth").toString();
            QString _year = _json_value.value("year").toString();
            QString _hour = _json_value.value("hour").toString();
            QString _minute = _json_value.value("minute").toString();
            QString _second = _json_value.value("second").toString();
//            debug("day >> " + _day);
//            debug("month >> " + _month);
//            debug("year >> " + _year);
//            debug("hour >> " + _hour);
//            debug("minute >> " + _minute);
//            debug("second >> " + _second);
            String _date = _year + _month + _day;
            String _time = _hour + ":" + _minute + ":" + _second;
            logDebug->setDateTime(_date, _time);
//            debug("getTimeZone >> Success");
            debug("Sync Time >> " + _date + " " + _time);
            try_synctime_cnt = 0;
            last_time_syntime = QDateTime::currentDateTime().toString("dd");
            cloud_box_ready = true;
            if(!STATIC_BOOL_WAIT_TO_REBOOT){
                emit signalConnectServerOK();//Start to check api buffer
                emit signalSetLEDServer(_LED_ON);
//                QTimer::singleShot(15000,this,SLOT(slotUploadLogFile()));//Test
            }
            emit signalResetClientTimeoutTime();
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
//        debug("# setAPIData");
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
//                debug("data >> " + _json_doc.toJson(QJsonDocument::Compact));
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

void SM_CIRBOX_CLOUD_API::setMachineType(QString _machine_type)
{
    debug("setMachineType >> "+ _machine_type);
    machine_type = _machine_type;
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

void SM_CIRBOX_CLOUD_API::slotCreatRunAppScript(QString _service_id)
{
//    cmdRemove_RunAppScript();
    if(_service_id == _SERVICE_MC){
        String _command = "sudo echo /Users/kitdev/CloudBoxApp/laundryBox.app > /Users/kitdev/CloudBoxApp/runapp.sh";
        system(_command.toStdString().c_str());
    }
    else{
        String _command = "sudo echo /Users/kitdev/CloudBoxApp/cloudBox.app > /Users/kitdev/CloudBoxApp/runapp.sh";
        system(_command.toStdString().c_str());
    }

    cmdChmodX_RunAppScript();

}

void SM_CIRBOX_CLOUD_API::slotClientPing(void)
{
    if(!cloud_box_ready){
        clientPingResetTimer();
        return;
    }

    if(STATIC_BOOL_WAIT_TO_REBOOT)
        return;

    client_ping_pulling_timer->stop();

    if(ethernet->moduleCannotUse()){
        debug("@2");
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//        clientPingResetTimer();
        return;
    }

    if(flag_wait_to_read_response){
        clientPingResetTimer();
        return;
    }

    if(!ethernet->internet->isConnect()){
        if(!ethernet->internet->connect()){
            //debug("Can't connect internet!!");
            emit signalSetLEDServer(_LED_OFF);
            debug("@3");
            cloud_box_ready = false;
            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//            clientPingResetTimer();
            return;
        }
    }

    String _url = String(_CIRBOX_CLOUD_URL);
    _url += "/api/v1/ping?";

    String _ccid = ethernet->sim->getCCID();
    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
        _ccid = "-";

    String _sq = String::number(ethernet->network->getSignalQuality(),10);
    if(_sq.length() == 0 || _sq.length() > 5)
        _sq = "-";

    _url += "serialno=" + serialno;
    _url += "&ccid=" + _ccid;
    _url += "&sq=" + _sq;

//    debug("#URL = " + _url);

    if(!ethernet->http->setURL(_url)){
//        ethernet->internet->resetConnecting();
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
        clientPingResetTimer();
        emit signalSetLEDServer(_LED_OFF);
        return;
    }
    if(ethernet->http->getMethod(true) < 0){
//        ethernet->internet->resetConnecting();
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
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
    try_synctime_cnt++;

    if(try_synctime_cnt > 5){
        debug("@9");
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
        try_synctime_cnt = 0;
        return;
    }

    if(!cloud_box_ready)
    {
        if(!ethernet->internet->isConnect()){
            debug("#A1");
            if(ethernet->internet->connect()){
                debug("#A2");
            }
        }

        if(!syncTime("+7")){
            debug("#A3");
            QTimer::singleShot(5000,this,SLOT(slotSyncTime()));
            debug("#A4");
        }
//        else{
//            ethernet->internet->disConnect();
//        }
    }
}

void SM_CIRBOX_CLOUD_API::slotGetTimeFromServer(void)
{
    if(flag_wait_to_read_response){
        return;
    }

    if(!ethernet->internet->isConnect()){
        if(!ethernet->internet->connect())
        {
            cloud_box_ready = false;
            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
            return;
        }
    }

    syncTime("+7");
}

bool SM_CIRBOX_CLOUD_API::slotUploadLogFile(String _date)// _date format --> ddMMyyyy
{
    if(!cloud_box_ready){
        return 0;
    }

    if(STATIC_BOOL_WAIT_TO_REBOOT){
        return 0;
    }

    debug("slotUploadLogFile");

//    debug("#1");

    clientPingResetTimer();

//    debug("#2");

    if(ethernet->moduleCannotUse()){
        debug("@4");
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//        clientPingResetTimer();
        return 0;
    }

//    debug("#3");

    if(flag_wait_to_read_response){
        return 0;
    }

//    debug("#4");

    if(!ethernet->internet->isConnect()){
        if(!ethernet->internet->connect()){
            //debug("Can't connect internet!!");
            emit signalSetLEDServer(_LED_OFF);
            debug("@5");
            cloud_box_ready = false;
            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//            clientPingResetTimer();
            return 0;
        }
    }

//    debug("#5");

    String _url = "ftp.cirbox.co.th";
    uint16_t _port = 77;
    String _user = "cirbox";
    String _pass = "Cir1213!";

//    String _url = String(_CIRBOX_CLOUD_URL);
//    _url += "/api/v1/ping?";

//    String _ccid = ethernet->sim->getCCID();
//    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
//        _ccid = "-";

//    String _sq = String::number(ethernet->network->getSignalQuality(),10);
//    if(_sq.length() == 0 || _sq.length() > 5)
//        _sq = "-";

//    _url += "serialno=" + serialno;
//    _url += "&ccid=" + _ccid;
//    _url += "&sq=" + _sq;

//    debug("#URL = " + _url);
    if(!ethernet->ftp->setContextid(1)){
        debug("Set coontextid - Unsuccess!!");
        return 0;
    }

    if(!ethernet->ftp->setAccount(_user,_pass)){
        debug("Set User or Password - Unsuccess!!");
        return 0;
    }

    if(!ethernet->ftp->setFiletype(0)){
        debug("Set file type - Unsuccess!!");
        return 0;
    }

    if(!ethernet->ftp->setTransmode(1)){
        debug("Set transmode - Unsuccess!!");
        return 0;
    }

    if(!ethernet->ftp->setTimeout(180)){
        debug("Set timeout - Unsuccess!!");
        return 0;
    }

    if(!ethernet->ftp->loginToServer(_url,_port,true)){
//        if(!ethernet->internet->resetConnecting()){
            cloud_box_ready = false;
            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//        }
//        debug("Login to FTP Server - Unsuccess!!");
//        clientPingResetTimer();
        return 0;
    }

    if(!ethernet->ftp->setCurrentDir("/",true)){
        debug("Set Path - Unsuccess!!");
        return 0;
    }

    if(!ethernet->ftp->getStatusFTPService(true)){
        if(!ethernet->ftp->logoutFromServer(true)){
//            if(!ethernet->internet->resetConnecting()){
                cloud_box_ready = false;
                QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//            }
        }
        return 0;
    }

//    String _path = "/media/sdcard/";
//    QFile _log_file(_path + "log.zip");
    String _path = "/media/sdcard/log/" + _date;
    QFile _log_file(_path + "/001.log");
    debug("## " + _path + "/001.log");

    ethernet->ftp->uploadFile(_date + ".txt",&_log_file);

    if(!ethernet->ftp->logoutFromServer(true)){
//        if(!ethernet->internet->resetConnecting()){
            cloud_box_ready = false;
            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//        }
//        debug("Logout from FTP Server - Unsuccess!!");
//        clientPingResetTimer();kill
        return 0;
    }

    clientPingResetTimer();
}

bool SM_CIRBOX_CLOUD_API::slotPostLog()
{
    if(!cloud_box_ready){
        return 0;
    }
//    debug("#1");
    if(ethernet->moduleCannotUse()){
        debug("@6");
        cloud_box_ready = false;
        ethernet->slotResetNetwork();
        return 0;
    }
//    debug("#2");
    if(flag_wait_to_read_response){
        return 0;
    }
//    debug("#3");
    if(!ethernet->internet->isConnect()){
        if(!ethernet->internet->connect()){
            cloud_box_ready = false;
            emit signalSetLEDServer(_LED_OFF);
            debug("@7");
            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
        }
        return 0;
    }
//    debug("#4");
    if(STATIC_BOOL_WAIT_TO_REBOOT){
        return 0;
    }
//    debug("#5");
    clientPingResetTimer();

    String _url = String(_CIRBOX_CLOUD_URL);
    _url += "/api/v1/log?";
    _url += "serialno=" + serialno;

    String _file_name = "001.log";
    String _path_file = "/media/sdcard/log/" + date_get_log + "/" + _file_name;

//    String _path_file = "/media/sdcard/log.zip";
    QFile _log_file(_path_file);

    if(!_log_file.exists()){
        debug("File not found!!");
        return 0;
    }
    if(!_log_file.open(QIODevice::ReadOnly)){
        debug("File can't open!!");
        return 0;
    }
    if(_log_file.isOpen())
        _log_file.close();


//    debug("#6 >> " + _path_file);

    if(!ethernet->http->setURL(_url)){
        //debug("#A1");
//        ethernet->internet->resetConnecting();
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
        emit signalSetLEDServer(_LED_OFF);
        return 0;
    }
    if(ethernet->http->postMethod(&_log_file,160,80) != _HTTP_STATUS_OK){
        //debug("#A2");
//        ethernet->internet->resetConnecting();
        cloud_box_ready = false;
        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
        emit signalSetLEDServer(_LED_OFF);
        return 0;
    }

    flag_wait_to_read_response = true;
    QTimer::singleShot(_READ_METHOD_DELAY_TIME,this,SLOT(slotReadResponseAPI()));
    return 1;
}
