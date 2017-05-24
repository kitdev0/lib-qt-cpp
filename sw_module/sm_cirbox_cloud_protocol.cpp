#include "sm_cirbox_cloud_protocol.h"

SM_CIRBOX_CLOUD_PROTOCOL::SM_CIRBOX_CLOUD_PROTOCOL(SM_CIRBOX_CLOUD_API *api_port, QObject *parent):
    QObject(parent)
{
#ifdef _CLOUD_BOX_PROTOCOL_DEBUG
    logDebug = new SM_DEBUGCLASS("Cloud-Protocol");
#endif
    api = api_port;
    cb_serial_port = new QSerialPort;
    client_online_time = new QTimer;
    check_comport_timer = new QTimer;

    connect(client_online_time,SIGNAL(timeout()),this,SLOT(slotCheckClientTimeout()));

    connect(cb_serial_port,SIGNAL(readyRead()),this,SLOT(slotReadSerialPort()));
    connect(this,SIGNAL(signalReadCBProtocol(String)),this,SLOT(slotReadCBProtocol(String)));
    connect(this,SIGNAL(signalReturnError()),this,SLOT(slotReturnError()));
    connect(this,SIGNAL(signalReturnOK()),this,SLOT(slotReturnOK()));
    connect(this,SIGNAL(signalReturnSuccess()),this,SLOT(slotReturnSuccess()));
    connect(this,SIGNAL(signalGetDataValue(String)),this,SLOT(slotGetDataValue(String)));
    connect(this,SIGNAL(signalSetDataValue(String)),this,SLOT(slotSetDataValue(String)));
    connect(check_comport_timer,SIGNAL(timeout()),this,SLOT(slotCheckComport()));
//    connect(cb_serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this,SLOT(slotSerialError(QSerialPort::SerialPortError)));
}

SM_CIRBOX_CLOUD_PROTOCOL::~SM_CIRBOX_CLOUD_PROTOCOL()
{
#ifdef _CLOUD_BOX_PROTOCOL_DEBUG
    delete logDebug;
#endif
    delete api;
    delete cb_serial_port;
}

void SM_CIRBOX_CLOUD_PROTOCOL::debug(String data)
{
#ifdef _CLOUD_BOX_PROTOCOL_DEBUG
#if _CLOUD_BOX_PROTOCOL_DEBUG == _DEBUG_SAY_ONLY
    logDebug->sayln(data);
#elif _CLOUD_BOX_PROTOCOL_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _CLOUD_BOX_PROTOCOL_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}

void SM_CIRBOX_CLOUD_PROTOCOL::sendData(String _data)
{
    QByteArray _byte = _data.toUtf8();
    cb_serial_port->write(_byte.constData(),_byte.length());
    if(cb_serial_port->bytesToWrite() > 0)
        cb_serial_port->flush();
}

bool SM_CIRBOX_CLOUD_PROTOCOL::tryToConnect(void)
{
    const auto infos = QSerialPortInfo::availablePorts();
    QStringList _port_list;

//    if(flag_first_scanning){
////        debug("Serial port scanning...");
//        flag_first_scanning = false;
//    }

    for (const QSerialPortInfo &info : infos) {
#ifdef Q_OS_OSX
        if(info.portName().indexOf(String(_OSX_SERIAL_PORT_DEVICE)) != -1){
            debug(info.portName());
            _port_list << info.portName();
        }
#else
#ifdef Q_OS_LINUX
        if(info.portName().indexOf(String(_RPI_USB_SERIAL_PORT_DEVICE)) != -1  || info.portName().indexOf(String(_RPI_ARDUINO_SERIAL_PORT_DEVICE)) != -1)
        {
//            debug(info.portName());
            _port_list << info.portName();
        }
#endif
#endif
    }

    if(_port_list.size() == 0){
//        if(flag_first_serial_not_found){
//            debug("Can't find serial port device!!");
//            flag_first_serial_not_found = false;
//        }
        if(!check_comport_timer->isActive())
            check_comport_timer->start(_CHECK_COMPORT_TIMER);
        return 0;
    }

    for(int i=0; i < _port_list.size(); i++)
    {
//        if(_port_list[i] != api->ethernet->module->serial_port->portName())
//        {
        if(cb_serial_port->isOpen())
            cb_serial_port->close();

        cb_serial_port->setPortName(_port_list[i]);
        cb_serial_port->setBaudRate(cb_serial_baud);
        cb_serial_port->setParity(QSerialPort::NoParity);

        if (cb_serial_port->open(QIODevice::ReadWrite)){
            //cb_serial_port->setDataTerminalReady(true);
            cb_serial_port->setDataTerminalReady(true);
            debug("Port name : " + _port_list[i]);
            debug("Open port >> passed");
            connect(cb_serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this,SLOT(slotSerialError(QSerialPort::SerialPortError)));
//            last_mid_no = -1;
//            connect(api,SIGNAL(signalResponseAPISuccess()),this,SLOT(slotReturnSuccess()));
//            connect(api,SIGNAL(signalResponseAPIUnsuccess()),this,SLOT(slotReturnUnsuccess()));
            return true;
        }
        else{
            debug("Port name : " + _port_list[i]);
            debug("Open port >> failed");
            debug("Error : " + cb_serial_port->errorString());
//            if(cb_serial_port->isOpen())
//                cb_serial_port->close();
            check_comport_timer->start(_CHECK_COMPORT_TIMER);
        }
//        }
    }
    return false;
}

//private

//--------- private slot --------//
void SM_CIRBOX_CLOUD_PROTOCOL::slotReadSerialPort(void)
{
    if(SM_CIRBOX_CLOUD_API::STATIC_BOOL_WAIT_TO_REBOOT){
        cb_serial_port->clear(cb_serial_port->AllDirections);
        return;
    }

    while(cb_serial_port->canReadLine())
    {
        String _str = cb_serial_port->readLine();
//        debug("Read from client >> " + _str);
        if(_str.indexOf('#') != -1){
            char index1 = _str.indexOf('#');
            char index2 = _str.indexOf("CB+") - 1;
            String _cb_no =  _str.mid(index1 + 1, index2-index1);
            current_mid = _cb_no.toInt();
//            if(_cb_no.toInt() == last_mid_no){
////                debug("#123");
//                return;
//            }
//            else{
//                last_mid_no = _cb_no.toInt();
//            debug("Receive mid = " + _cb_no);
//            }
        }

        if(_str.indexOf("CB+") != -1){
            char index1 = _str.indexOf('+');
            char index2 = _str.indexOf('\r') - 1;
            String _cb_data =  _str.mid(index1 + 1, index2-index1);
            client_online_time->start(_CLIENT_TIMEOUT_TIME);
            emit signalReadCBProtocol(_cb_data);
            emit signalSetLEDClient(_LED_ON);
            if(machine_client_connect_ok != 1)
            {
                QJsonDocument _json_report;
                QJsonObject _json_object;

                machine_client_connect_ok = 1;
                _json_object.insert("table_no","1");
                _json_object.insert("api01","ONLINE");

                _json_report.setObject(_json_object);

                debug("Machine client >> ONLINE");

                emit signalReportDataToCloud(&_json_report);
            }
        }
    }
//    CB+SET@T1:API1=99\r\n
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReadCBProtocol(String _str)
{ 
    if(SM_CIRBOX_CLOUD_API::STATIC_BOOL_WAIT_TO_REBOOT){
        return;
    }
//    debug(">> " + _str);
    if(_str.indexOf(cmd.UPDATE) != -1){
//        debug("#CMD1");
        if(current_mid != last_mid){
            api->reqUpdateAPIData();
            last_mid = current_mid;
//            debug("mid = " + String::number(last_mid));
        }
        slotReturnMID(current_mid);
    }
    else if(_str.indexOf(cmd.CLEAR) != -1){
//        debug("#CMD2");
        if(current_mid != last_mid){
            api->reqClrJsonAPIData();
            last_mid = current_mid;
//            debug("mid = " + String::number(last_mid));
        }
        slotReturnMID(current_mid);
    }
    else if(_str.indexOf(cmd.SET) != -1){
//        debug("#CMD3");
        if(current_mid != last_mid){
            emit signalSetDataValue(_str);
            last_mid = current_mid;
//            debug("mid = " + String::number(last_mid));
        }
        slotReturnMID(current_mid);

    }
    else if(_str.indexOf(cmd.GET) != -1){
//        debug("#CMD4");
        if(current_mid != last_mid){
            emit signalGetDataValue(_str);
            last_mid = current_mid;
        }
    }
    else if(_str.indexOf(cmd.STATE) != -1){
//        debug("# << cmd.STATE");
        if(!flag_client_is_connected){
            flag_client_is_connected = true;
            emit signalClientISConnect();
        }
        slotReturnOK();
    }
    else if(_str.indexOf(cmd.READY) != -1){
//        debug("# << cmd.READY");
        if(api->getCloudBoxReady()){
//            debug("# << CloudBox Ready");
            slotReturnReady();
        }
        else{
//            debug("# << CloudBox Not Ready !!");
            slotReturnBusy();
        }
    }
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnMID(int16_t _mid)
{
//    debug("<< OK");
    String _str = String::number(_mid,10) + "\r";
    sendData(_str);
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnOK()
{
//    debug("<< OK");
    sendData("OK\r");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnReady()
{
//    debug("<< OK");
    sendData("READY\r");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnBusy()
{
//    debug("<< OK");
    sendData("BUSY\r");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnError()
{
    sendData("ERROR\r");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnSuccess()
{
//    debug("Return >> SUCCESS\r\n");
//    sendData("SC\r\n");
    String _str = "#\r";
//    + String::number(last_mid_no,10) + "\r\n";
    sendData(_str);
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnUnsuccess()
{
    sendData("UNSUCCESS\r");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotGetDataValue(String _str)
{

}

void SM_CIRBOX_CLOUD_PROTOCOL::slotSetDataValue(String _str)
{
    bool ok;

    char index1 = _str.indexOf("@");
    char index2 = _str.indexOf(":") - 1;
    String _table_no =  _str.mid(index1 + 1, index2-index1);

    index1 = _str.indexOf(":");
    index2 = _str.indexOf("=") - 1;
    String _api_no =  _str.mid(index1 + 1, index2-index1);

    index1 = _str.indexOf("=");
    index2 = _str.indexOf('\r') - 1;
    String _value =  _str.mid(index1 + 1, index2-index1);

    uint8_t _table_id = _table_no.toInt(&ok);
    if(_table_id > _TABLE_NO_MAX || !ok){
//        emit signalReturnError();
        return;
    }

    uint8_t _api_id = _api_no.toInt(&ok);
    if(_api_id > _API_ID_MAX || !ok){
//        emit signalReturnError();
        return;
    }

    if(_api_id < 10){
        _api_no = "0" + _api_no;
    }

//    debug("table no. = " + _table_no);
//    debug("api no. = " + _api_no);
//    debug("value = " + _value);sudo

    api->setAPIData(_table_no,"api" + _api_no,_value);

//    if(api->setAPIData(_table_no,"api" + _api_no,_value)){
//        emit signalReturnSuccess();
//    }
//    else{
//        emit signalReturnError();
//    }
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotCheckClientTimeout()
{
    if(machine_client_connect_ok != 0)
    {
        QJsonDocument _json_report;
        QJsonObject _json_object;

        machine_client_connect_ok = 0;
        _json_object.insert("table_no","1");
        _json_object.insert("api01","OFFLINE");

        _json_report.setObject(_json_object);

        debug("Machine client >> OFFLINE");

        if(cb_serial_port->isOpen())
            cb_serial_port->close();
        if(!check_comport_timer->isActive())
            check_comport_timer->start(_CHECK_COMPORT_TIMER);

        emit signalReportDataToCloud(&_json_report);
    }
    emit signalSetLEDClient(_LED_OFF);
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotCheckComport()
{
    if(check_comport_timer->isActive())
        check_comport_timer->stop();
    tryToConnect();
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotSerialError(QSerialPort::SerialPortError _error)
{
//    if (_error == QSerialPort::ResourceError || _error == QSerialPort::UnknownError) {
//        debug("Port name : " + cb_serial_port->portName() + " >> lost Connection!!");
//        debug("Error : " + cb_serial_port->errorString());
//        cb_serial_port->close();
//        check_comport_timer->start(_CHECK_COMPORT_TIMER);
//        flag_first_scanning = true;
//        flag_first_serial_not_found = true;
//    }

    if(_error == last_usb_err){
        return;
    }

    last_usb_err = _error;

    if(_error == QSerialPort::NoError){
        debug("#NoError");
        return;
    }

//    if(_error == QSerialPort::OpenError){
//        debug("#OpenError");
//        return;
//    }

//    if(_error == QSerialPort::ReadError){
//        debug("#ReadError");
//        return;
//    }
    if(_error == QSerialPort::NotOpenError){
        debug("#NotOpenError");
        return;
    }

    switch (_error)
    {
//    case QSerialPort::NoError:
////        debug("#NoError");
//        break;//return
    case QSerialPort::ReadError:
        debug("#ReadError");
        break;
//    case QSerialPort::NotOpenError:
////        debug("#NotOpenError");
//        break;
    case QSerialPort::OpenError:
        debug("#OpenError");
        break;
    case QSerialPort::DeviceNotFoundError:
        debug("#DeviceNotFoundError");
        break;
    case QSerialPort::PermissionError:
        debug("#PermissionError");
        break;
    case QSerialPort::ParityError:
        debug("#ParityError");
        break;
    case QSerialPort::FramingError:
        debug("#FramingError");
        break;
    case QSerialPort::BreakConditionError:
        debug("#BreakConditionError");
        break;
    case QSerialPort::WriteError:
        debug("#WriteError");
        break;
    case QSerialPort::ResourceError:
        debug("#ResourceError");
        break;
    case QSerialPort::UnsupportedOperationError:
        debug("#UnsupportedOperationError");
        break;
    case QSerialPort::UnknownError:
        debug("#UnknownError");
        break;
    case QSerialPort::TimeoutError:
        debug("#TimeoutError");
        break;
    default:
        debug("#ErrorNotDefine");
        break;
    }

//    if(cb_serial_port->isOpen())
//        cb_serial_port->close();
    disconnect(cb_serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this,SLOT(slotSerialError(QSerialPort::SerialPortError)));

    if(!check_comport_timer->isActive()){
        check_comport_timer->start(_CHECK_COMPORT_TIMER);
    }

//    if(cb_serial_port->isOpen()){
//        debug("Port name : " + cb_serial_port->portName() + " >> lost Connection!!");
//        debug("Error : " + cb_serial_port->errorString());
//        cb_serial_port->close();
//    }
////    if(!check_comport_timer->isActive())
////        check_comport_timer->start(_CHECK_COMPORT_TIMER);
//    flag_first_scanning = true;
//    flag_first_serial_not_found = true;


//    if(_error == )
//    debug("Port name : " + cb_serial_port->portName() + " >> lost Connection!!");
//    debug("Error : " + cb_serial_port->errorString());
//    //cb_serial_port->disconnect(SLOT(slotSerialError(QSerialPort::SerialPortError)));
//    //disconnect(cb_serial_port,0,0,0);
//    //disconnect(cb_serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this,SLOT(slotSerialError(QSerialPort::SerialPortError)));
//    if(cb_serial_port->isOpen())
//        cb_serial_port->close();
//    check_comport_timer->start(_CHECK_COMPORT_TIMER);
    //disconnect()
}

// private slot //

void SM_CIRBOX_CLOUD_PROTOCOL::slotResetClientTimeoutTime()
{
    if(client_online_time->isActive())
        client_online_time->start(_CLIENT_TIMEOUT_TIME);
}

//--------- public --------//
bool SM_CIRBOX_CLOUD_PROTOCOL::begin(uint32_t _baud)
{
//    debug("## begin ##");
    is_begin = true;
    cb_serial_baud = _baud;
    client_online_time->start(_CLIENT_TIMEOUT_TIME);
#ifdef _CB_SERIAL_PORT_NAME
    cb_serial_port->setPortName(_CB_SERIAL_PORT_NAME);
    cb_serial_port->setBaudRate(cb_serial_baud);
    cb_serial_port->setParity(QSerialPort::NoParity);
    if (cb_serial_port->open(QIODevice::ReadWrite)){
        debug("Port name : " + String(_CB_SERIAL_PORT_NAME));
        debug("Open port >> passed");
        //connect(serial_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        connect(api,SIGNAL(signalResponseAPISuccess()),this,SLOT(slotReturnSuccess()));
        connect(api,SIGNAL(signalResponseAPIUnsuccess()),this,SLOT(slotReturnUnsuccess()));
        return true;
    }
    else{
        debug("Port name : " + String(_CB_SERIAL_PORT_NAME));
        debug("Open port >> failed");
        debug("Error : " + cb_serial_port->errorString());
        cb_serial_port->close();
        return false;
    }
#else
    return tryToConnect();
#endif
    return false;
}

// public //
