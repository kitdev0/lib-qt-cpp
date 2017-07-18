#include "sm_cirbox_cloud_api.h"

SM_CIRBOX_CLOUD_API::SM_CIRBOX_CLOUD_API(SM_GSM_MODULE *my_ethernet,QObject *parent) :
    QObject(parent)
{
    logDebug = new SM_DEBUGCLASS("Cloud-API");
    ethernet = my_ethernet;
//    check_api_buff_to_send_timer = new QTimer;
    client_ping_pulling_timer = new QTimer;
//    check_flag_post_log_timer = new QTimer;
//    systime_timer = new QTimer;

    json_api_data = new QJsonObject;


    connect(this, SIGNAL(signalStartLoopApp()),this,SLOT(slotAppLoop()));
//    connect(check_api_buff_to_send_timer, SIGNAL(timeout()),this,SLOT(slotCheckAPIBuffToSend()));
    connect(client_ping_pulling_timer,SIGNAL(timeout()),this,SLOT(slotSetFlagPing()));
    connect(ethernet,SIGNAL(signalInternetIsOK()),this,SLOT(slotSyncTime()));
//    connect(ethernet,SIGNAL(signalResetNetworkIsOk()),this,SLOT(slotSyncTime()));
//    connect(check_flag_post_log_timer, SIGNAL(timeout()),this,SLOT(slotPostLog()));
    connect(this,SIGNAL(signalTryToInitModule(uint8_t)),this,SLOT(slotTryToInitModule(uint8_t)));

}

SM_CIRBOX_CLOUD_API::~SM_CIRBOX_CLOUD_API()
{
    delete logDebug;
    delete ethernet;
    delete json_api_data;
    delete client_ping_pulling_timer;
//    delete check_api_buff_to_send_timer;
//    delete check_flag_post_log_timer;
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

String SM_CIRBOX_CLOUD_API::getMachineTime(void)
{
    String _time = logDebug->currentDay() + " " + logDebug->currentTime();
    return _time;
}

void SM_CIRBOX_CLOUD_API::checkToExecuteCmd(uint8_t _cmd)
{
    switch (_cmd)
    {
    case _CMD_SHUTDOWN://shutdown
        debug("!! Shutdown !!");
        emit signalOffLEDAll();
        flag_set_led_off_all = true;
        QTimer::singleShot(4000,this,SLOT(slotUnmountSDCard()));
        QTimer::singleShot(5000,this,SLOT(slotCloudBoxShutdown()));
        connect_server_ready = false;
        STATIC_BOOL_WAIT_TO_REBOOT = true;
        break;
    case _CMD_REBOOT://reboot
        debug("!! Reboot !!");
        emit signalOffLEDAll();
        flag_set_led_off_all = true;
        QTimer::singleShot(4000,this,SLOT(slotUnmountSDCard()));
        QTimer::singleShot(5000,this,SLOT(slotCloudBoxReboot()));
        connect_server_ready = false;
        STATIC_BOOL_WAIT_TO_REBOOT = true;
        break;
    case _CMD_GET_LOG://get log
//        debug("Req - Logfile");
        if(date_get_log.size() > 0){
            debug("!! Get Log !!");
//            check_flag_post_log_timer->start(_CHECK_TO_POST_LOG_TIME);
            flag_to_post_log = true;
//            QTimer::singleShot(2000,this,SLOT(slotPostLog()));
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
    case _CMD_RESET_MACHINE:
        flag_cmd_reset_board = true;
        emit signalResetMachine();
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
//        cmdRemove_interfaceFile();

        if(_service_id == _SERVICE_SC){ //Single Client
//            debug("Set system to Single Client");

//            slotCreatRunAppScript(_SERVICE_SC);

            String _command = "sudo update-rc.d mosquitto disable";
            system(_command.toStdString().c_str());

            _command = "sudo cp /Users/kitdev/CloudBoxApp/interfaces.sc /etc/network/interfaces";
            system(_command.toStdString().c_str());
        }

        if(_service_id == _SERVICE_MC){ //Multi Client
//            debug("Set system to Multi Client");

//            slotCreatRunAppScript(_SERVICE_MC);

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

void SM_CIRBOX_CLOUD_API::responseAPIIncorrect()
{
    ethernet->module->clrSerialBuffer();

    response_api_incorrect_cnt++;
    if(response_api_incorrect_cnt > 5){
        response_api_incorrect_cnt = 0;
        flag_api_response_incorrect = true;
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

//void SM_CIRBOX_CLOUD_API::slotStartToCheckAPIBuff(void)
//{
//    check_api_buff_to_send_timer->start(_CHECK_API_BUFF_TO_SEND_TIMER);
//}

//void SM_CIRBOX_CLOUD_API::slotStopToCheckAPIBuff(void)
//{
//    if(check_api_buff_to_send_timer->isActive())
//        check_api_buff_to_send_timer->stop();
//}

//void SM_CIRBOX_CLOUD_API::slotCheckAPIBuffToSend(void)
//{
//    checkAPIBuffToSend();
//}



//void SM_CIRBOX_CLOUD_API::slotCheckAPIBuffToSend(void)
//{
////    if(!connect_server_ready){
////        return;
////    }

////    if(ethernet->moduleCannotUse()){
////        debug("@8");
////        connect_server_ready = false;
//////        ethernet->slotResetGsmModule();
//////        ethernet->slotResetNetwork();
////        return;
////    }

//    slotStopToCheckAPIBuff();

//    if(STATIC_BOOL_WAIT_TO_REBOOT){
//        return;
//    }

//    if(flag_wait_to_read_response){
//        slotStartToCheckAPIBuff();
//        return;
//    }

//    if(!apiDataToSendAvailable()){
//        return;
//    }

//    if(!json_api_buff[last_api_buff].isEmpty()){
//        QJsonDocument _json_doc(json_api_buff[last_api_buff]);
//        if(reportData(&_json_doc)){
//            slotStartToCheckAPIBuff();
//        }
//    }
//    else{
//        last_api_buff = (uint16_t)(last_api_buff + 1) % _API_BUFFER_SIZE;
//        debug("data >> empty!!");
//    }

//}

void SM_CIRBOX_CLOUD_API::slotReadResponseAPI(void)
{
    String _api = ethernet->http->readMethod(true);

    if(_api == "-1"){//Error
        try_to_read_response++;
        if(try_to_read_response < 3){
            QTimer::singleShot(3000,this,SLOT(slotReadResponseAPI()));
            return;
        }
        try_to_read_response = 0;
        flag_wait_to_read_response = false;
        return;
    }

    if(_api == "-2"){//Timeout
        try_to_read_response = 0;
        flag_wait_to_read_response = false;
        return;
    }

    if(_api.length() > 0)
    {
        debug("Response : " + _api);
        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());
        uint16_t _response_status = responseStatus(&_json_response);
        String _response_message = responseMessage(&_json_response);
        uint8_t _response_cmd = responseCmd(&_json_response);
        if(responseCmdData(&_json_response).length() > 0)
            date_get_log = responseCmdData(&_json_response);
        QString _response_id = responseDeviceID(&_json_response);

        if(_response_status == _HTTP_STATUS_OK || _response_status == _HTTP_STATUS_ACCEPTED)
        {
            response_api_incorrect_cnt = 0;
            flag_api_response_incorrect = false;

            if(_response_message == _MESSAGE_SUCCESS){
                if(flag_report_data){
                    last_api_buff = (uint16_t)(last_api_buff + 1) % _API_BUFFER_SIZE;
                    flag_report_data = false;
                }

//                checkResponseIDToChangeService(_response_id);

                if(STATIC_BOOL_WAIT_TO_REBOOT != true && _response_cmd > 0){
                    checkToExecuteCmd(_response_cmd);
                }
                connect_server_ready = true;
                flag_deactiveted = false;
                emit signalSetLEDServer(_LED_ON);
            }
            else if(_response_message == _MESSAGE_DEACTIVATED){
                debug("!! Warning : This ClouBox Device is Deactivated");
                flag_deactiveted = true;
            }
        }
        else{
            responseAPIIncorrect();
            debug("!! Warning : Response >> API is Incorrect");
        }
    }
    else{
        responseAPIIncorrect();
        debug("!! Warning : Response >> Data is empty!!");
    }

    try_to_read_response = 0;
    flag_wait_to_read_response = false;
    return;

}

void SM_CIRBOX_CLOUD_API::slotReadResponseAPIClientPing()
{
    String _api = ethernet->http->readMethod(true);

    if(_api == "-1"){//Error
        try_to_read_response++;
        debug("try_to_read_response = " + String(try_to_read_response));
        if(try_to_read_response < 3){
            QTimer::singleShot(3000,this,SLOT(slotReadResponseAPI()));
            return;
        }
        try_to_read_response = 0;
        flag_wait_to_read_response = false;
        return;
    }

    if(_api == "-2"){//Timeout
        try_to_read_response = 0;
        flag_wait_to_read_response = false;
        return;
    }

    if(_api.length() > 0)
    {
//        debug("Response [ping] >> " + _api);
        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());
        uint16_t _response_status = responseStatus(&_json_response);
        String _response_message = responseMessage(&_json_response);
        uint8_t _response_cmd = responseCmd(&_json_response);
        if(responseCmdData(&_json_response).length() > 0)
            date_get_log = responseCmdData(&_json_response);
        QString _response_id = responseDeviceID(&_json_response);

        if(_response_status == _HTTP_STATUS_OK)
        {
            response_api_incorrect_cnt = 0;
            flag_api_response_incorrect = false;

            if(_response_message == _MESSAGE_SUCCESS){
//                checkResponseIDToChangeService(_response_id);
                if(STATIC_BOOL_WAIT_TO_REBOOT != true && _response_cmd > 0){
                    checkToExecuteCmd(_response_cmd);
                }
                connect_server_ready = true;
                flag_deactiveted = false;
                emit signalSetLEDServer(_LED_ON);
            }
            else if(_response_message == _MESSAGE_DEACTIVATED){
                debug("!! Warning : This ClouBox Device is Deactivated");
                flag_deactiveted = true;
            }
        }
        else{
            responseAPIIncorrect();
            debug("!! Warning : Response [ping] >> API is Incorrect");
        }

    }
    else{
        responseAPIIncorrect();
        debug("!! Warning : Response [ping] >> Data is empty!!");
    }
    flag_wait_to_read_response = false;
}

void SM_CIRBOX_CLOUD_API::slotTryToInitModule(uint8_t _try_cnt)
{
//    ethernet->internet->disConnect();
    emit signalSetLEDServer(_LED_OFF);
    connect_server_ready = false;
    if(!setDisInternet()){
        QTimer::singleShot(5000,ethernet,SLOT(slotTryToSetPwrOFF()));
        return;
    }

    if(_try_cnt < 2){
        QTimer::singleShot(2000,ethernet,SLOT(slotInitModule()));
        return;
    }

    if(_try_cnt < 5){
        QTimer::singleShot(5*60000,ethernet,SLOT(slotInitModule()));
        return;
    }

    if(_try_cnt < 10){
        QTimer::singleShot(15*60000,ethernet,SLOT(slotInitModule()));
        return;
    }
    QTimer::singleShot(30*60000,ethernet,SLOT(slotInitModule()));
}

void SM_CIRBOX_CLOUD_API::slotSyncTime(void)
{
    int8_t _res = syncTime("+7",false);

    if(_res == 1){ //Success
        try_synctime_cnt = 0;
        try_to_slot_synctime_cnt = 0;
        if(STATIC_BOOL_WAIT_TO_REBOOT != true){
            emit signalSetLEDServer(_LED_ON);
        }
        emit signalStartLoopApp();           
        resetPingTime();
    }
    else if(_res == -1){ //api incorrect
        if(flag_api_response_incorrect){
            emit signalStartLoopApp();
        }
        else{
            ethernet->internet->disConnect();
            QTimer::singleShot(3000,this,SLOT(slotSyncTime()));
        }
    }
    else{ //Can't connect to server
        try_to_slot_synctime_cnt++;
        if(try_to_slot_synctime_cnt > 5){
            try_to_slot_synctime_cnt = 0;
            emit signalTryToInitModule(try_synctime_cnt++);
        }
        else{
            ethernet->internet->disConnect();
            QTimer::singleShot(3000,this,SLOT(slotSyncTime()));
        }
    }
}

void SM_CIRBOX_CLOUD_API::slotTryPingToServer()
{
    debug("## Try to ping to server [" + String::number(try_to_ping_to_server) + "]...");

    if(!clientPing()){
        debug("getMetod ping >> Unsuccess");
        try_ping_again++;
        if(try_ping_again > 5){
            try_ping_again = 0;
            emit signalTryToInitModule(try_ping_cnt++);
        }
        else{
            ethernet->internet->disConnect();
            QTimer::singleShot(3000,this,SLOT(slotTryPingToServer()));
        }
        return;
    }
    else{
        try_to_ping_to_server++;
        debug("getMetod ping >> Success");
        slotWaitResponsePing();
    }
}

void SM_CIRBOX_CLOUD_API::slotWaitResponsePing()
{
    if(STATIC_BOOL_WAIT_TO_REBOOT){
        return;
    }

    if(flag_wait_to_read_response){
        QTimer::singleShot(2000,this,SLOT(slotWaitResponsePing()));
        return;
    }

    if(connect_server_ready && !flag_api_response_incorrect){//response api success
        try_to_ping_to_server = 0;
        flag_synctime_success = false;
        QTimer::singleShot(2000,this,SLOT(slotSyncTime()));
        return;
    }

    if(try_to_ping_to_server > 12){
        try_to_ping_to_server = 0;
        QTimer::singleShot(5000,ethernet,SLOT(slotInitModule()));
        return;
    }
    else{
        ethernet->internet->disConnect();
        debug("Wait to ping to server again [5 min].....");
        QTimer::singleShot(5*60000,this,SLOT(slotTryPingToServer()));
        return;
    }
}

void SM_CIRBOX_CLOUD_API::slotAppLoop()
{
    if(STATIC_BOOL_WAIT_TO_REBOOT){
        return;
    }

    if(flag_wait_to_read_response){
        QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
        return;
    }

    if(flag_deactiveted || flag_api_response_incorrect){
        connect_server_ready = false;
        emit signalSetLEDServer(_LED_OFF);
        QTimer::singleShot(5000,this,SLOT(slotTryPingToServer()));
        return;
    }

    if(apiDataToSendAvailable()){
        if(!json_api_buff[last_api_buff].isEmpty()){
            QJsonDocument _json_doc(json_api_buff[last_api_buff]);
            debug("## Update data [" + String::number(last_api_buff) +"]...");
            int8_t _value = reportData(&_json_doc); //Post Report
            if(_value == 1){//report success
                QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
                resetPingTime();
                return;
            }
            if(_value == 0){//report timeout
                try_post_report_again++;
                debug("!! Warning : Report data >> Faile [" + String::number(try_post_report_again) + "]");
                if(try_post_report_again > 5){
                    try_post_report_again = 0;
                    emit signalTryToInitModule(try_post_report_cnt++);
                }
                else{
                    if(setDisInternet())
                        QTimer::singleShot(_APP_LOOP_TIME + 5000,this,SLOT(slotAppLoop()));
                    else
                        QTimer::singleShot(5000,ethernet,SLOT(slotTryToSetPwrOFF()));

                }
                emit signalSetLEDServer(_LED_OFF);
                return;
            }
            if(_value == -1){//data is incorrect
                debug("!! Warning : Report data >> Data is incorrect");
                last_api_buff = (uint16_t)(last_api_buff + 1) % _API_BUFFER_SIZE;
                QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
                return;
            }
        }
        else{
            debug("data >> empty!!");
            last_api_buff = (uint16_t)(last_api_buff + 1) % _API_BUFFER_SIZE;
        }
        QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
        return;
    }

    if(flag_post_log_unsuccess){
//        debug("## Post Log [Unsuccess Info.] ##");
        if(!postLog(post_log_error_info)){
            debug("Post Log [Unsuccess Info.] >> Unsuccess");
//            ethernet->internet->disConnect();
            if(setDisInternet()){
                QTimer::singleShot(_APP_LOOP_TIME + 5000,this,SLOT(slotAppLoop()));
            }
            else{
                post_log_error_info = "";
                flag_post_log_unsuccess = false;
                QTimer::singleShot(5000,ethernet,SLOT(slotTryToSetPwrOFF()));
            }
            emit signalSetLEDServer(_LED_OFF);
            return;
        }
        debug("Post Log [Unsuccess Info.] >> Success");
        resetPingTime();
        post_log_error_info = "";
        flag_post_log_unsuccess = false;
        QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
        return;
    }
    else if(flag_to_post_log)
    {
        flag_to_post_log = false;
//        debug("## Post Log ##");

        int8_t _value = postLog();

        if(_value == 1){//post success
            debug("Post Log >> Success");
            resetPingTime();
            QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
            return;
        }
        else{
            debug("Post Log >> Unsuccess");
            flag_post_log_unsuccess = true;
            if(_value == 0){//post log timeout
//                ethernet->internet->disConnect();
                if(setDisInternet())
                    QTimer::singleShot(_APP_LOOP_TIME + 5000,this,SLOT(slotAppLoop()));
                else
                    QTimer::singleShot(5000,ethernet,SLOT(slotTryToSetPwrOFF()));
                emit signalSetLEDServer(_LED_OFF);
            }
            else{//data is incorrect
                QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
            }
            return;
        }
    }

    if(QDateTime::currentDateTime().toString("dd") != last_time_syntime){
//        debug("## Synctime ##");
        if(syncTime("+7",true)){
            resetPingTime();
            debug("Synctime >> Success");
        }
        else{
            debug("Synctime >> Unsuccess");
        }
        QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
        return;
    }

    if(flag_to_ping){
//        debug("## Client ping...");
        flag_to_ping = false;
        resetPingTime();
        if(!clientPing()){
            try_to_client_ping++;
            debug("!! Warning : Client ping >> Unsuccess [" + String::number(try_to_client_ping) + "]");
            if(try_to_client_ping > 5){
                try_to_client_ping = 0;
                emit signalTryToInitModule(try_client_ping_cnt++);
            }else{
                if(setDisInternet())
                    QTimer::singleShot(_APP_LOOP_TIME + 5000,this,SLOT(slotAppLoop()));
                else
                    QTimer::singleShot(5000,ethernet,SLOT(slotTryToSetPwrOFF()));
            }
            emit signalSetLEDServer(_LED_OFF);
            return;
        }
//        debug(" >> Success");
        QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));
        return;
    }

    QTimer::singleShot(_APP_LOOP_TIME,this,SLOT(slotAppLoop()));

}

void SM_CIRBOX_CLOUD_API::slotSetFlagPing()
{
    flag_to_ping = true;
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

bool SM_CIRBOX_CLOUD_API::reqUpdateAPIData(void)
{
//    if(!ethernet->internetIsReady()){
//        reqClrJsonAPIData();
//        return 0;
//    }
    if(!json_api_data->isEmpty())
    {
        debug("## Req. update data [" + String::number(api_buff) +"]");
        json_api_data->insert("m_time",getMachineTime());
        json_api_buff[api_buff] = *json_api_data;
        api_buff = (uint16_t)(api_buff + 1) % _API_BUFFER_SIZE;
        reqClrJsonAPIData();
//        slotCheckAPIBuffToSend();
        return 1;
    }
    else{
        debug("!! Warning : Req. update data >> Unsuccess [Json data >> empty]");
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

//void SM_CIRBOX_CLOUD_API::clientPingResetTimer()
//{
//    client_ping_pulling_timer->start(_CLIENT_PING_PULLING_TIME);
//}

void SM_CIRBOX_CLOUD_API::setMachineType(QString _machine_type)
{
    debug("setMachineType >> "+ _machine_type);
    machine_type = _machine_type;
}

bool SM_CIRBOX_CLOUD_API::flagCmdResetBoard()
{
   bool _flag = flag_cmd_reset_board;
   flag_cmd_reset_board = false;
   return _flag;
}

bool SM_CIRBOX_CLOUD_API::syncTime(String _gmt,bool _flag_sync_again)
{
    uint8_t _loop_try_read = 0;
    debug("## getTimeZone ##");

    if(!_flag_sync_again && flag_synctime_success){
        debug("flag_synctime_success = 1");
        return 1;
    }

    String _url = "";
    String _api = "";
    String _ccid = "";
    String _sq = "";
    String _header = "";

    _ccid = ethernet->sim->getCCID();
    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
        _ccid = "-";

    _sq = String::number(ethernet->network->getSignalQuality(),10);
    if(_sq.length() == 0 || _sq.length() > 5)
        _sq = "-";

    _url = "/api/v1/synctime?";
    _url += "serialno=" + serialno;
    _url += "&gmt=" + _gmt;
    _url += "&ccid=" + _ccid;
    _url += "&sq=" + _sq;

//    debug("#URL = " + _url);

    _header += "GET " + _url + " HTTP/1.1\r\n";
    _header += "Host: " + String(_CIRBOX_HOST) + "\r\n";
    _header += "Accept: */*\r\n";
    _header += "User-Agent: QUECTEL_MODULE\r\n";
    _header += "Connection: Closed\r\n";
    _header += "Content-Type:application/json\r\n";
    _header += "firmware-version:" + String::number(_FIRMWARE_VERSION) + "\r\n";
    _header += "token:";
    _header += "Jgrorg53*540Jnck";
    _header += "\r\n";
    _header += "\r\n";


    if(!ethernet->internet->isConnect())
    {
        debug("!! Warning : Internet don't connect");
        if(!ethernet->internet->connect())
        {
            debug("syncTime : Connect to internet >> Unsuccess!!");
            return 0;
        }
    }

    if(_url.length() == 0){
        return 0;
    }
    else if(!ethernet->http->setURL(String(_CIRBOX_CLOUD_URL) + _url)){
        debug("syncTime : setURL >> Unsuccess !!");
        return 0;
    }

    if(ethernet->http->getMethodCustomHeader(&_header,_header.length(),80) < 0){
        debug("syncTime : getMethod >> Unsuccess !!");
        return 0;
    }

LOOP_READ_METHOD:
    SM_DELAY::delay_ms(2000);
    _api = ethernet->http->readMethod(true);

    if(_api == "-1"){ //Error
        _loop_try_read++;
        if(_loop_try_read < 3)
            goto LOOP_READ_METHOD;
        else
            return 0;
    }

    if(_api == "-2"){//timeout
        return 0;
    }

    if(_api.length() > 0)
    {
        debug("Response [synctime] >> " + _api);
        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());
        uint16_t _response_status = responseStatus(&_json_response);
        String _response_message = responseMessage(&_json_response);
        uint8_t _response_cmd = responseCmd(&_json_response);
        if(responseCmdData(&_json_response).length() > 0)
            date_get_log = responseCmdData(&_json_response);
        QString _response_id = responseDeviceID(&_json_response);

        if(_response_status == _HTTP_STATUS_OK)
        {
            response_api_incorrect_cnt = 0;
            flag_api_response_incorrect = false;

            if(_response_message == _MESSAGE_SUCCESS){
//                checkResponseIDToChangeService(_response_id);

                if(STATIC_BOOL_WAIT_TO_REBOOT != true && _response_cmd > 0){
                    checkToExecuteCmd(_response_cmd);
                }

                QJsonObject _json_value = _json_response.object().value("result").toObject();
                QString _day = _json_value.value("day").toString();
                QString _month = _json_value.value("mounth").toString();
                QString _year = _json_value.value("year").toString();
                QString _hour = _json_value.value("hour").toString();
                QString _minute = _json_value.value("minute").toString();
                QString _second = _json_value.value("second").toString();
                String _date = _year + _month + _day;
                String _time = _hour + ":" + _minute + ":" + _second;
                logDebug->setDateTime(_date, _time);
                debug("Sync Time >> " + _date + " " + _time);
                flag_synctime_success = true;
                connect_server_ready = true;
                flag_deactiveted = false;
                last_time_syntime = QDateTime::currentDateTime().toString("dd");
                emit signalSetLEDServer(_LED_ON);
                return 1;
            }
            else if(_response_message == _MESSAGE_DEACTIVATED){
                debug("!! Warning : This ClouBox Device is Deactivated");
                flag_deactiveted = true;
                return 1;
            }
        }
        else{
            responseAPIIncorrect();
            debug("!! Warning : Response [synctime] >> API is Incorrect");
            return -1;
        }
        ///---///
    }
    else{
        responseAPIIncorrect();
        debug("!! Warning : Response [synctime] >> Data is empty!!");
        return -1;
    }
}

//bool SM_CIRBOX_CLOUD_API::syncTime(String _gmt)
//{
//    debug("## getTimeZone ##");

//    if(flag_synctime_success){
//        return 1;
//    }

//    String _url = "";
//    String _api = "";
//    uint8_t _cnt = 0;

//    String _ccid = ethernet->sim->getCCID();
//    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
//        _ccid = "-";

//    String _sq = String::number(ethernet->network->getSignalQuality(),10);
//    if(_sq.length() == 0 || _sq.length() > 5)
//        _sq = "-";

//    _url = String(_CIRBOX_CLOUD_URL);
//    _url += "/api/v1/synctime?";
//    _url += "serialno=" + serialno;
//    _url += "&gmt=" + _gmt;
//    _url += "&ccid=" + _ccid;
//    _url += "&sq=" + _sq;

////    debug("#URL = " + _url);
////    if(!connectInternet()){
////        return 0;
////    }

////    if(!httpSetURL(_url)){
////        return 0;
////    }

//    if(!httpGetMethod(_url)){
//        emit signalSetLEDServer(_LED_OFF);
//        return 0;
//    }

//READ_METHOD:
//    SM_DELAY::delay_ms(2000);
//    _api = ethernet->http->readMethod(true);
//    if(_api == "-2"){
//        return 0;
//    }
//    else if(_api == "-1"){ //Error
//        _cnt++;
//        if(_cnt < 3)
//            goto READ_METHOD;
//        else
//            return 0;
//    }
//    else if(_api.length() == 0){

//    }

//    if(_api.length() > 0)
//    {
////        debug("response api >> " + _api);

//        QJsonDocument _json_response = QJsonDocument::fromJson(_api.toUtf8());
//        uint8_t _response_cmd = responseCmd(&_json_response);
//        date_get_log = responseCmdData(&_json_response);
//        QString _response_id = responseDeviceID(&_json_response);

//        checkResponseIDToChangeService(_response_id);

//        if(_response_cmd > 0){
////            debug("Response Cmd >> " + String::number(_response_cmd,10));
//            checkToExecuteCmd(_response_cmd);
//        }

//        if(responseStatus(&_json_response) != _HTTP_STATUS_OK || responseMessage(&_json_response) != _MESSAGE_SUCCESS){
//            debug(_json_response.toJson(QJsonDocument::Compact));
//            debug("Sync Time - Json Response error");
//            return 0;
//        }
//        else{
//            QJsonObject _json_value = _json_response.object().value("result").toObject();
//            QString _day = _json_value.value("day").toString();
//            QString _month = _json_value.value("mounth").toString();
//            QString _year = _json_value.value("year").toString();
//            QString _hour = _json_value.value("hour").toString();
//            QString _minute = _json_value.value("minute").toString();
//            QString _second = _json_value.value("second").toString();
////            debug("day >> " + _day);
////            debug("month >> " + _month);
////            debug("year >> " + _year);
////            debug("hour >> " + _hour);
////            debug("minute >> " + _minute);
////            debug("second >> " + _second);
//            String _date = _year + _month + _day;
//            String _time = _hour + ":" + _minute + ":" + _second;
//            logDebug->setDateTime(_date, _time);
////            debug("getTimeZone >> Success");
//            debug("Sync Time >> " + _date + " " + _time);
//            flag_synctime_success = true;
//            last_time_syntime = QDateTime::currentDateTime().toString("dd");
////            connect_server_ready = true;
//            if(!STATIC_BOOL_WAIT_TO_REBOOT){
//                emit signalSetLEDServer(_LED_ON);
////                QTimer::singleShot(15000,this,SLOT(slotUploadLogFile()));//Test
//            }
//            emit signalResetClientTimeoutTime();
//            return 1;
//        }
//    }
//    else{
//        debug("getTimeZone >> Sync Time - API Response >> Empty");
//        debug("getTimeZone >> Unsuccess !!");
//    }
////    connect_server_ready = false;
//    return 0;
//}

//public slot
void SM_CIRBOX_CLOUD_API::slotReqUpdateAPI(QJsonDocument *_json_report)
{
    QJsonObject _json_object = _json_report->object();

    if(!_json_object.isEmpty())
    {
        _json_object.insert("m_time",getMachineTime());
        json_api_buff[api_buff] = _json_object;
        api_buff = (uint16_t)(api_buff + 1) % _API_BUFFER_SIZE;
//        slotCheckAPIBuffToSend();
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

int8_t SM_CIRBOX_CLOUD_API::reportData(QJsonDocument *_json_report)
{
//    debug("## reportData ##");

    String _table_no = _json_report->object().value(String("table_no")).toString();
    if(_table_no.toInt() > _TABLE_NO_MAX || _table_no.toInt() < 0){
        debug("Warning >> table-no is incorrect !!");
        return -1;
    }

    String _sub_url = "";

    if(_table_no.size() < 2)
        _sub_url = "table0" + _table_no;
    else
        _sub_url = "table" + _table_no;

    String _url = String(_API_V1_VENDING_URL) + _sub_url;
    String _api = "";
    String _data = "";
    String _header = "";

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
    debug("Payload >> " + _api);

    _header += "POST " + _url + " HTTP/1.1\r\n";
    _header += "Host: " + String(_CIRBOX_HOST) + "\r\n";
    _header += "Accept: */*\r\n";
    _header += "User-Agent: CIRBOX_CLOUDBOX\r\n";
    _header += "Connection: Closed\r\n";
    _header += "Content-Type:application/json\r\n";
    _header += "Content-Length: ";
    _header += String::number(_api.length()) + "\r\n";
    _header += "firmware-version:" + String::number(_FIRMWARE_VERSION) + "\r\n";
    _header += "token:";
    _header += "Jgrorg53*540Jnck";
    _header += "\r\n";
    _header += "\r\n";

    _data = _header+_api;

    if(!ethernet->internet->isConnect()){
        debug("!! Warning : Internet don't connect");
        if(!ethernet->internet->connect()){
//            debug("# Can't connect internet!!");
            return 0;
        }
    }

    if(_url.length() == 0){
        debug("!!Warning >> URL is empry.");
        return -1;
    }

    if(!ethernet->http->setURL(String(_CIRBOX_CLOUD_URL) + _url)){
        return 0;
    }

    if(_api.length() == 0){
        debug("!!Warning >> API is empry.");
        return -1;
    }

    if(ethernet->http->postMethod(&_data) != _HTTP_STATUS_OK){
        return 0;
    }

    flag_report_data = true;
    flag_wait_to_read_response = true;
    QTimer::singleShot(_READ_METHOD_DELAY_TIME,this,SLOT(slotReadResponseAPI()));
    return 1;
}

bool SM_CIRBOX_CLOUD_API::clientPing(void)
{
    String _url = "/api/v1/ping?";
    String _header = "";

    String _ccid = ethernet->sim->getCCID();
    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
        _ccid = "-";

    String _sq = String::number(ethernet->network->getSignalQuality(),10);
    if(_sq.length() == 0 || _sq.length() > 5)
        _sq = "-";

    _url += "serialno=" + serialno;
    _url += "&ccid=" + _ccid;
    _url += "&sq=" + _sq;

    _header += "GET " + _url + " HTTP/1.1\r\n";
    _header += "Host: " + String(_CIRBOX_HOST) + "\r\n";
    _header += "Accept: */*\r\n";
    _header += "User-Agent: QUECTEL_MODULE\r\n";
    _header += "Connection: Closed\r\n";
    _header += "Content-Type:application/json\r\n";
    _header += "firmware-version:" + String::number(_FIRMWARE_VERSION) + "\r\n";
    _header += "token:";
    _header += "Jgrorg53*540Jnck";
    _header += "\r\n";
    _header += "\r\n";

    if(_url.length() == 0){
        return 0;
    }

//    debug("#URL = " + _url);

    if(!ethernet->internet->isConnect()){
        debug("!! Warning : Internet don't connect");
        if(!ethernet->internet->connect()){
            return 0;
        }
    }
//    debug("#A");
    if(!ethernet->http->setURL(String(_CIRBOX_CLOUD_URL) + _url)){
        return 0;
    }
//    debug("#B");
    if(ethernet->http->getMethodCustomHeader(&_header,_header.length(),80) < 0){
        return 0;
    }
//    debug("#C");
    flag_wait_to_read_response = true;
    QTimer::singleShot(_READ_METHOD_DELAY_TIME,this,SLOT(slotReadResponseAPIClientPing()));
    return 1;

}

//void SM_CIRBOX_CLOUD_API::slotGetTimeFromServer(void)
//{
//    if(flag_wait_to_read_response){
//        QTimer::singleShot(5000,this,SLOT(slotGetTimeFromServer()));
//        return;
//    }

//    if(!ethernet->internet->isConnect()){
//        if(!ethernet->internet->connect())
//        {
//            //connect_server_ready = false;
////            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//            return;
//        }
//    }

//    syncTime("+7");
//}

//bool SM_CIRBOX_CLOUD_API::slotUploadLogFile(String _date)// _date format --> ddMMyyyy
//{
////    if(!connect_server_ready){
////        return 0;
////    }

//    if(STATIC_BOOL_WAIT_TO_REBOOT){
//        return 0;
//    }

//    debug("slotUploadLogFile");

////    debug("#1");

//    clientPingResetTimer();

////    debug("#2");

////    if(ethernet->moduleCannotUse()){
////        debug("@4");
////        connect_server_ready = false;
////        QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
//////        clientPingResetTimer();
////        return 0;
////    }

////    debug("#3");

//    if(flag_wait_to_read_response){
//        return 0;
//    }

////    debug("#4");

//    if(!ethernet->internet->isConnect()){
//        if(!ethernet->internet->connect()){
//            //debug("Can't connect internet!!");
//            emit signalSetLEDServer(_LED_OFF);
//            debug("@5");
////            connect_server_ready = false;
////            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
////            clientPingResetTimer();
//            return 0;
//        }
//    }

////    debug("#5");

//    String _url = "ftp.cirbox.co.th";
//    uint16_t _port = 77;
//    String _user = "cirbox";
//    String _pass = "Cir1213!";

////    String _url = String(_CIRBOX_CLOUD_URL);
////    _url += "/api/v1/ping?";

////    String _ccid = ethernet->sim->getCCID();
////    if(_ccid.length() == 0 || _ccid.length() > _CCID_LEN_MAX)
////        _ccid = "-";

////    String _sq = String::number(ethernet->network->getSignalQuality(),10);
////    if(_sq.length() == 0 || _sq.length() > 5)
////        _sq = "-";

////    _url += "serialno=" + serialno;
////    _url += "&ccid=" + _ccid;
////    _url += "&sq=" + _sq;

////    debug("#URL = " + _url);
//    if(!ethernet->ftp->setContextid(1)){
//        debug("Set coontextid - Unsuccess!!");
//        return 0;
//    }

//    if(!ethernet->ftp->setAccount(_user,_pass)){
//        debug("Set User or Password - Unsuccess!!");
//        return 0;
//    }

//    if(!ethernet->ftp->setFiletype(0)){
//        debug("Set file type - Unsuccess!!");
//        return 0;
//    }

//    if(!ethernet->ftp->setTransmode(1)){
//        debug("Set transmode - Unsuccess!!");
//        return 0;
//    }

//    if(!ethernet->ftp->setTimeout(180)){
//        debug("Set timeout - Unsuccess!!");
//        return 0;
//    }

//    if(!ethernet->ftp->loginToServer(_url,_port,true)){
////        if(!ethernet->internet->resetConnecting()){
////            connect_server_ready = false;
////            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
////        }
////        debug("Login to FTP Server - Unsuccess!!");
////        clientPingResetTimer();
//        return 0;
//    }

//    if(!ethernet->ftp->setCurrentDir("/",true)){
//        debug("Set Path - Unsuccess!!");
//        return 0;
//    }

//    if(!ethernet->ftp->getStatusFTPService(true)){
//        if(!ethernet->ftp->logoutFromServer(true)){
////            if(!ethernet->internet->resetConnecting()){
////                connect_server_ready = false;
////                QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
////            }
//        }
//        return 0;
//    }

////    String _path = "/media/sdcard/";
////    QFile _log_file(_path + "log.zip");
//    String _path = "/media/sdcard/log/" + _date;
//    QFile _log_file(_path + "/001.log");
//    debug("## " + _path + "/001.log");

//    ethernet->ftp->uploadFile(_date + ".txt",&_log_file);

//    if(!ethernet->ftp->logoutFromServer(true)){
////        if(!ethernet->internet->resetConnecting()){
////            connect_server_ready = false;
////            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
////        }
////        debug("Logout from FTP Server - Unsuccess!!");
////        clientPingResetTimer();kill
//        return 0;
//    }

////    clientPingResetTimer();

//    return 1;
//}

//void SM_CIRBOX_CLOUD_API::slotPostLog()
//{
//    debug("# slotPostLog #");

//    if(check_flag_post_log_timer->isActive()){
//        check_flag_post_log_timer->stop();
//    }

//    if(!postLog()){
//        check_flag_post_log_timer->start(_CHECK_TO_POST_LOG_TIME);
//    }
//}

int8_t SM_CIRBOX_CLOUD_API::postLog()
{
//    if(STATIC_BOOL_WAIT_TO_REBOOT){
//        debug("!! Warning >> Post log is unsuccess [STATIC_BOOL_WAIT_TO_REBOOT]");
//        return 0;
//    }

//    if(flag_wait_to_read_response){
//        debug("!! Warning >> Post log is unsuccess [flag_wait_to_read_response]");
//        return 0;
//    }
//    debug("#3");
//    debug("## postLog ##");

    String _header = "";
    String _url = "/api/v1/log?";
    _url += "serialno=" + serialno;

    String _file_name = "001.log";
    String _path_file = "/media/sdcard/log/" + date_get_log + "/" + _file_name;

//    String _path_file = "/media/sdcard/log.zip";
    QFile _log_file(_path_file);

    if(!_log_file.exists()){
        debug("!! Warning >> Post log is unsuccess [File not found!!]");
        post_log_error_info = "!! Warning >> Post log is unsuccess [File not found!!]";
        return -1;
    }

    if(!_log_file.open(QIODevice::ReadOnly)){
        debug("!! Warning >> Post log is unsuccess [File can't open!!]");
        post_log_error_info = "!! Warning >> Post log is unsuccess [File can't open!!]";
        return -1;
    }

    QByteArray _my_data;

    if(_log_file.isOpen()){
        _my_data = _log_file.readAll();
        _log_file.close();
    }

    _header += "POST " + _url + " HTTP/1.1\r\n";
    _header += "Host: " + String(_CIRBOX_HOST) + "\r\n";
    _header += "Accept: */*\r\n";
    _header += "User-Agent: CIRBOX_CLOUDBOX\r\n";
    _header += "Connection: Closed\r\n";
    _header += "Content-Type:application/json\r\n";
    _header += "Content-Length: ";
    _header += String::number(_my_data.length()) + "\r\n";
    _header += "firmware-version:" + String::number(_FIRMWARE_VERSION) + "\r\n";
    _header += "token:";
    _header += "Jgrorg53*540Jnck";
    _header += "\r\n";
    _header += "\r\n";


//    debug("#6 >> " + _path_file);

    if(!ethernet->internet->isConnect()){
        debug("!! Warning : Internet don't connect");
        if(!ethernet->internet->connect()){
//            connect_server_ready = false;
//            debug("!! Warning >> Post log is unsuccess [Can't connect internet]");
//            emit signalSetLEDServer(_LED_OFF);
//            debug("@7");
//            QTimer::singleShot(5000,ethernet,SLOT(slotResetNetwork()));
            debug("!! Warning >> Post log is unsuccess [Can't Open internet]");
            post_log_error_info = "!! Warning >> Post log is unsuccess [Can't Open internet]";
            return 0;
        }
    }

    if(!ethernet->http->setURL(String(_CIRBOX_CLOUD_URL) + _url)){
        debug("!! Warning >> Post log is unsuccess [setURL Unsucces]");
        post_log_error_info = "!! Warning >> Post log is unsuccess [setURL Unsucces]";
        return 0;
    }
    if(ethernet->http->postMethodCustomHeader(&_header,&_log_file,120,120) != _HTTP_STATUS_OK){
        debug("!! Warning >> Post log is unsuccess [postMethod Unsuccess]");
        post_log_error_info = "!! Warning >> Post log is unsuccess [postMethod Unsuccess]";
        return 0;
    }

    flag_wait_to_read_response = true;
    QTimer::singleShot(_READ_METHOD_DELAY_TIME,this,SLOT(slotReadResponseAPI()));
    return 1;
}

bool SM_CIRBOX_CLOUD_API::postLog(String _str)
{
    String _data = "";
    String _url = "/api/v1/log?";
    _url += "serialno=" + serialno;

    String _header = "POST " + _url + " HTTP/1.1\r\n";
    _header += "Host: " + String(_CIRBOX_HOST) + "\r\n";
    _header += "Accept: */*\r\n";
    _header += "User-Agent: CIRBOX_CLOUDBOX\r\n";
    _header += "Connection: Closed\r\n";
    _header += "Content-Type:application/json\r\n";
    _header += "Content-Length: ";
    _header += String::number(_str.length()) + "\r\n";
    _header += "firmware-version:" + String::number(_FIRMWARE_VERSION) + "\r\n";
    _header += "token:";
    _header += "Jgrorg53*540Jnck";
    _header += "\r\n";
    _header += "\r\n";

    _data = _header + _str;

//    debug("#6 >> " + _path_file);

    if(!ethernet->internet->isConnect()){
        debug("!! Warning : Internet don't connect");
        if(!ethernet->internet->connect()){
            return 0;
        }
    }

    if(_url.length() == 0){
        return 0;
    }

    if(!ethernet->http->setURL(String(_CIRBOX_CLOUD_URL) + _url)){
        return 0;
    }

    if(ethernet->http->postMethod(&_data) != _HTTP_STATUS_OK){
        return 0;
    }

    flag_wait_to_read_response = true;
    QTimer::singleShot(_READ_METHOD_DELAY_TIME,this,SLOT(slotReadResponseAPI()));
    return 1;
}

void SM_CIRBOX_CLOUD_API::resetPingTime()
{
    client_ping_pulling_timer->start(_CLIENT_PING_PULLING_TIME);
}

bool SM_CIRBOX_CLOUD_API::setDisInternet()
{
    debug("setDisInternet");
    for(int i=0; i < 5;i++){
        if(ethernet->internet->disConnect()){
//            connect_server_ready = true;
            return true;
        }
        SM_DELAY::delay_ms(2000);
    }
    connect_server_ready = false;
    return false;
}

