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
    connect(this,SIGNAL(signalGetDataValue(String)),this,SLOT(slotGetDataValue(String)));
    connect(this,SIGNAL(signalSetDataValue(String)),this,SLOT(slotSetDataValue(String)));
    connect(check_comport_timer,SIGNAL(timeout()),this,SLOT(slotCheckComport()));
    connect(cb_serial_port, SIGNAL(error(QSerialPort::SerialPortError)), this,SLOT(slotSerialError(QSerialPort::SerialPortError)));
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

    debug("Serial port scanning...");

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
            debug(info.portName());
            _port_list << info.portName();
        }
#endif
#endif
    }

    if(_port_list.size() == 0){
        debug("Can't find serial port device!!");
        check_comport_timer->start(_CHECK_COMPORT_TIMER);
        return 0;
    }

    for(int i=0; i < _port_list.size(); i++)
    {
//        if(_port_list[i] != api->ethernet->module->serial_port->portName())
//        {
        cb_serial_port->setPortName(_port_list[i]);
        cb_serial_port->setBaudRate(cb_serial_baud);
        cb_serial_port->setParity(QSerialPort::NoParity);
        if (cb_serial_port->open(QIODevice::ReadWrite)){
            //cb_serial_port->setDataTerminalReady(true);
            cb_serial_port->setDataTerminalReady(true);
            debug("Port name : " + _port_list[i]);
            debug("Open port >> passed");
            //connect(serial_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
            connect(api,SIGNAL(signalResponseAPISuccess()),this,SLOT(slotReturnSuccess()));
            connect(api,SIGNAL(signalResponseAPIUnsuccess()),this,SLOT(slotReturnUnsuccess()));
            return true;
        }
        else{
            debug("Port name : " + _port_list[i]);
            debug("Open port >> failed");
            debug("Error : " + cb_serial_port->errorString());
            cb_serial_port->close();
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
    while(cb_serial_port->canReadLine())
    {
        String _str = cb_serial_port->readLine();
        //debug("read data >> " +_str);
        if(_str.indexOf("CB+") != -1){
            char index1 = _str.indexOf('+');
            char index2 = _str.indexOf('\r') - 1;
            String _cb_data =  _str.mid(index1 + 1, index2-index1);
            client_online_time->start(_CLIENT_TIMEOUT_TIME);
            emit signalReadCBProtocol(_cb_data);
            emit signalSetLEDClient(_LED_ON);
            if(machine_client_connect_ok == 0)
            {
                QJsonDocument _json_report;
                QJsonObject _json_object;

                machine_client_connect_ok = 1;
                _json_object.insert("table_no","3");
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
//    debug(">> " + _str);
    if(_str.indexOf(cmd.UPDATE) != -1){
//        debug("#CMD1");
        if(api->reqUpdateAPIData())
            emit signalReturnOK();
        else
            emit signalReturnError();
    }
    else if(_str.indexOf(cmd.CLEAR) != -1){
//        debug("#CMD2");
        if(api->reqClrJsonAPIData())
            emit signalReturnOK();
        else
            emit signalReturnError();
    }
    else if(_str.indexOf(cmd.SET) != -1){
//        debug("#CMD3");
        emit signalSetDataValue(_str);
    }
    else if(_str.indexOf(cmd.GET) != -1){
//        debug("#CMD4");
        emit signalGetDataValue(_str);
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

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnOK()
{
//    debug("<< OK");
    sendData("OK\r\n");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnReady()
{
//    debug("<< OK");
    sendData("READY\r\n");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnBusy()
{
//    debug("<< OK");
    sendData("BUSY\r\n");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnError()
{
    sendData("ERROR\r\n");
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
        emit signalReturnError();
        return;
    }

    uint8_t _api_id = _api_no.toInt(&ok);
    if(_api_id > _API_ID_MAX || !ok){
        emit signalReturnError();
        return;
    }

    if(_api_id < 10){
        _api_no = "0" + _api_no;
    }

//    debug("table no. = " + _table_no);
//    debug("api no. = " + _api_no);
//    debug("value = " + _value);sudo

    if(api->setAPIData(_table_no,"api" + _api_no,_value)){
        emit signalReturnOK();
    }
    else{
        emit signalReturnError();
    }
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnSuccess()
{
    sendData("\r\nSUCCESS\r\n");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotReturnUnsuccess()
{
    sendData("\r\nUNSUCCESS\r\n");
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotCheckClientTimeout()
{
    if(machine_client_connect_ok != 0)
    {
        QJsonDocument _json_report;
        QJsonObject _json_object;

        machine_client_connect_ok = 0;
        _json_object.insert("table_no","3");
        _json_object.insert("api01","OFFLINE");

        _json_report.setObject(_json_object);

        debug("Machine client >> OFFLINE");

        emit signalReportDataToCloud(&_json_report);
    }
    emit signalSetLEDClient(_LED_OFF);
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotCheckComport()
{
    check_comport_timer->stop();
    tryToConnect();
}

void SM_CIRBOX_CLOUD_PROTOCOL::slotSerialError(QSerialPort::SerialPortError _error)
{
    if (_error == QSerialPort::ResourceError) {
        debug("Port name : " + cb_serial_port->portName() + " >> lost Connection!!");
        debug("Error : " + cb_serial_port->errorString());
        cb_serial_port->close();
        check_comport_timer->start(_CHECK_COMPORT_TIMER);
    }
}

// private slot //

//--------- public --------//
bool SM_CIRBOX_CLOUD_PROTOCOL::begin(uint32_t _baud)
{
    debug("## begin ##");
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
