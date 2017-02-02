#include "hm_uc20.h"
#include "../../../../../lib-qt-cpp/sw_module/sm_delay.h"

QElapsedTimer timer0;

HM_UC20CLASS::HM_UC20CLASS(QObject *parent) :
    QObject(parent)
{
#ifdef _UC20_DEBUG
    logDebug = new SM_DEBUGCLASS("UC20_MODULE");
#endif
}

HM_UC20CLASS::~HM_UC20CLASS()
{
#ifdef _UC20_DEBUG
    delete logDebug;
#endif
    serial_port->close();
    delete serial_port;
}

void HM_UC20CLASS::debug(String data)
{
#ifdef _UC20_DEBUG
#if _UC20_DEBUG == _DEBUG_SAY_ONLY
    logDebug->say(data);
#elif _UC20_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _UC20_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}

void HM_UC20CLASS::timeoutStart(void)
{
    timer0.start();
}

void HM_UC20CLASS::timeoutReset(void)
{
    timer0.start();
}

void HM_UC20CLASS::setPwrKeyPinActive(void)
{
    gpio.digitalWrite(pwr_key,_LOW);
    SM_DELAY::delay_ms(2000);
    gpio.digitalWrite(pwr_key,_HIGH);
}

bool HM_UC20CLASS::timeoutCheck(uint32_t _time)
{
    if (timer0.elapsed() > _time)
	{
		return 1;
	}
	return 0;
}

void HM_UC20CLASS::clrSerialBuffer(void)
{
	timeoutStart();
    serial_port->clear();
}

bool HM_UC20CLASS::tryToConnect(void)
{
    const auto infos = QSerialPortInfo::availablePorts();
    QStringList _port_list;

    serial_port = new QSerialPort();
    serial_port->setBaudRate(serial_baud);
    serial_port->setParity(QSerialPort::NoParity);

    debug("Serial port scanning...");

    for (const QSerialPortInfo &info : infos) {
#ifdef Q_OS_OSX
        if(info.portName().indexOf(String(_OSX_SERIAL_PORT_DEVICE)) != -1){
            debug(info.portName());
            _port_list << info.portName();
        }
#else
#ifdef Q_OS_LINUX
        if(info.portName().indexOf(String(_RPI_USB_SERIAL_PORT_DEVICE)) != -1){
            debug(info.portName());
            _port_list << info.portName();
        }
#endif
#endif
    }

    if(_port_list.size() == 0){
        debug("Can't find serial port device!!");
        return 0;
    }

    for(int i=0; i < _port_list.size(); i++)
    {
        serial_port->setPortName(_port_list[i]);
        if (serial_port->open(QIODevice::ReadWrite)){
            debug("Port name : " + _port_list[i]);
            debug("Open port >> passed");
            //connect(serial_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
            if(simpleCommand()){
                return true;
            }
            else{
                serial_port->close();
            }
        }
        else{
            debug("Port name : " + _port_list[i]);
            debug("Open port >> failed");
            debug("Error : " + serial_port->errorString());
            serial_port->close();
        }
    }
    return false;
}

bool HM_UC20CLASS::connectToGSMPort(void)
{
    debug("Connect to GSM port...");
    serial_port = new QSerialPort();
    String _port_name = "";
#ifdef _GSM_MODULE_SERIAL_PORT
    _port_name = String(_GSM_MODULE_SERIAL_PORT);
#else
    _port_name = "/dev/ttyO1";
#endif
    serial_port->setPortName(_port_name);
    serial_port->setBaudRate(serial_baud);
    serial_port->setParity(QSerialPort::NoParity);
    if (serial_port->open(QIODevice::ReadWrite)){
        debug("Port name : " + _port_name);
        debug("Open port >> passed");
//        connect(serial_port, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
//        serial_port->write("ATI\r\n");
#ifdef _SEND_SIMPLE_COMMAND
        if(simpleCommand()){
            return true;
        }
        else{
            serial_port->close();
            return false;
        }
#endif
        return true;
    }
    else{
        debug("Port name : " + _port_name);
        debug("Open port >> failed");
        debug("Error : " + serial_port->errorString());
        serial_port->close();
    }
    return false;
}

bool HM_UC20CLASS::simpleCommand(void) //AT
{
    sendData("ATI",1);
    debug("Send simple command");

    return waitOK(_WAIT_OK_TIMEOUT+1000);
}

bool HM_UC20CLASS::connectModule(void)
{

#ifdef _GSM_MODULE_SERIAL_PORT
    is_connected = true;//make to send
    if (connectToGSMPort()){
        debug("Module connected. ^_^");
        is_connected = true;
        return true;
    }
    else{
        debug("Can't connect module. T_T");
        is_connected = false;
        return false;
    }
#else
    is_connected = true;
    if(tryToConnect()){
        debug("Module connected. ^_^");
        is_connected = true;
        return true;
    }
    else{
        debug("Can't connect module. T_T");
        is_connected = false;
        return false;
    }
#endif
    return false;
}

void HM_UC20CLASS::serialSend(QByteArray _byte, bool _flag_ln)
{
    serial_port->write(_byte.constData(),_byte.length());
    if(_flag_ln)
        serial_port->write("\r\n");
    if(serial_port->bytesToWrite() > 0)
        serial_port->flush();
}
//protected


//public
bool HM_UC20CLASS::begin(uint32_t _baud, uint16_t _pwr_pin)
{
    debug("# begin");
    is_begin = true;
    serial_baud = _baud;
    pwr_key = _pwr_pin;

#ifdef Q_OS_LINUX
    gpio.pinExport(pwr_key);
    gpio.pinMode(pwr_key, _OUTPUT);
    gpio.digitalWrite(pwr_key,_LOW);
#endif
    return connectModule();
}

bool HM_UC20CLASS::begin(uint32_t _baud)
{
    return begin(_baud,_GSM_PWK_PIN);
}

bool HM_UC20CLASS::dataAvailable(void)
{
    if(serial_port->bytesAvailable())
        return 1;
    else
        return 0;
}

char HM_UC20CLASS::receiveData(void)
{
    QByteArray _data = serial_port->read(1);
    return _data[0];
}

bool HM_UC20CLASS::sendData(String _data, bool _flag_ln)
{
    if(is_connected)
    {
        QByteArray _byte = _data.toUtf8();
        serialSend(_byte,_flag_ln);
        return true;
    }
    else{
        debug("Module is don't connecting!!");
        return false;
    }
}

bool HM_UC20CLASS::sendData(int _value, bool _flag_ln)
{
    if(is_connected)
    {
        QByteArray _byte = String::number(_value,10).toUtf8();
        serialSend(_byte,_flag_ln);
        return true;
    }
    else{
        debug("Module is don't connecting!!");
        return false;
    }
}

String HM_UC20CLASS::receiveStringUntil(char _data)
{
    return receiveStringUntil(String(_data));
}

String HM_UC20CLASS::receiveStringUntil(String _data)
{
    String _str = "";
    if(!serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
        debug("No response!!");
        return "";
    }

    for(;serial_port->bytesAvailable();)
    {
        _str += serial_port->read(1);
        if (_str.indexOf(_data) != -1)
            return _str;
    }
    return "";
}

void HM_UC20CLASS::setPwrKeyPin(uint16_t _pin)
{
    debug("set Pwr-Pin >> " + String(_pin));
    if(is_begin)
        gpio.pinUnExport(pwr_key);
    pwr_key = _pin;
    gpio.pinExport(pwr_key);
    gpio.pinMode(pwr_key, _OUTPUT);
    gpio.digitalWrite(pwr_key, _LOW);
}

bool HM_UC20CLASS::setPwrKey(bool _value)
{
	uint8_t _tryCnt = 0;
	bool _flag = false;
	if(_value)
        debug("set Power Key >> ON");
	else
        debug("set Power Key >> OFF");
    setPwrKeyPinActive();

    while (1)
	{
        if (serial_port->canReadLine())
		{
            String _str = serial_port->readLine();
//			debug(_str);
            if (_str.indexOf("RDY") != -1)
			{
                debug("Power-ON");
                if (_value == _HIGH) {
					_flag = true;
					break;
				}
				else
                    setPwrKeyPinActive();
			}
            else if (_str.indexOf("POWERED DOWN") != -1)
			{
                debug("Power-OFF");
                if (_value == _LOW) {
					_flag = true;
					break;
				}
				else
                    setPwrKeyPinActive();
			}

		}
        else if(!serial_port->waitForReadyRead(10000)){
            debug("setPwr >> Response timeout!!");
            if (_tryCnt >= 1) {
                _flag = false;
                break;
            }
            else{
                setPwrKeyPinActive();
                timeoutReset();
                _tryCnt++;
            }
        }
	}
    return _flag;
}

void HM_UC20CLASS::setPwrOn()
{
    uint16_t _pwr_pin = _GSM_PWR_PIN;
    debug("set Power >> On");
    if(flag_power_pin_is_export)
        gpio.pinUnExport(_pwr_pin);
    gpio.pinExport(_pwr_pin);
    gpio.pinMode(_pwr_pin, _OUTPUT);
    gpio.digitalWrite(_pwr_pin,_HIGH);
    flag_power_pin_is_export = true;
}

void HM_UC20CLASS::setPwrOff()
{
    uint16_t _pwr_pin = _GSM_PWR_PIN;
    debug("set Power >> Off");
    if(flag_power_pin_is_export)
        gpio.pinUnExport(_pwr_pin);
    gpio.pinExport(_pwr_pin);
    gpio.pinMode(_pwr_pin, _OUTPUT);
    gpio.digitalWrite(_pwr_pin,_LOW);
    flag_power_pin_is_export = true;
}

bool HM_UC20CLASS::waitToReady(uint32_t _wait_time)
{
	while (1)
	{
        if(serial_port->canReadLine())
        {
            String _str = serial_port->readLine();
//            debug("read data >> " + _str);

            if (_str.indexOf("PB DONE") != -1)
			{
                debug(" Ready...");
				return 1;
			}
            else if (_str.indexOf("POWERED DOWN") != -1)
			{
                setPwrKey(_HIGH);
			}
		}
        else if(!serial_port->waitForReadyRead(_wait_time)){
            debug("waitToReady >> Response timeout!!");
            return 0;
        }
	}
}

bool HM_UC20CLASS::waitOK(uint32_t _time)
{
	bool _res = false;
	_res = waitOK(_time, true);
	return _res;
}

bool HM_UC20CLASS::waitOK_ndb(uint32_t _time)
{
	bool _res = false;
	_res = waitOK(_time, false);
	return _res;
}

bool HM_UC20CLASS::waitOK(uint32_t _time, bool _ndb)
{
    while(1)
    {
        if(serial_port->canReadLine())
        {
            String _str = serial_port->readLine();
            //debug("read data >> " +_str);
            if(_str.indexOf("OK") != -1){
//                if (_ndb)
//                    debug("<< " + _str);
                return 1;
            }
            if(_str.indexOf("ERROR") != -1){
                if (_ndb)
                    debug("<< " + _str);
                return 0;
            }
//            timeoutStart();
        }
        else if(!serial_port->waitForReadyRead(_time)){
            if (_ndb)
                debug("waitOK >> Response timeout!!");
            return 0;
        }
    }
}



bool HM_UC20CLASS::setURCPort(URC_t _port)		//QURCCFG=_port
{
	if (_port == _URC_USB_AT) {
        if(!sendData("AT+QURCCFG=\"urcport\",\"usbat\"",1))
            return 0;
        debug("set URC-Port >> USB-AT");
	}
	else if (_port == _URC_USB_MODEM) {
        if(!sendData("AT+QURCCFG=\"urcport\",\"usbmodem\"",1))
            return 0;
        debug("set URC-Port >> USB-MODEM");
	}
	else if(_port == _URC_UART1){
        if(!sendData("AT+QURCCFG=\"urcport\",\"uart1\"",1))
            return 0;
        debug("set URC-Port >> UART1");
	}
	
    return waitOK_ndb(_WAIT_OK_TIMEOUT);
}

bool HM_UC20CLASS::setEcho(bool _value)			//ATEn
{
    if (_value) {
        if(!sendData("ATE1",1))
            return 0;
        debug("set Echo >> ON");
    }
    else {
        if(!sendData("ATE0",1))
            return 0;
        debug("set Echo >> OFF");
    }

    return waitOK(_WAIT_OK_TIMEOUT);
}

bool HM_UC20CLASS::setPhoneFunc(uint8_t _value)	//CFUN=_value
{
    String _str = "AT+CFUN=" + String::number(_value,10);
    if(!sendData(_str,1))
        return 0;
    debug(">> " + _str);
	//debug("set Phone Func. >> " + String(_value));

    return waitOK(10000);
}

URC_t HM_UC20CLASS::getURCPort(void)			//QURCCFG?
{
    debug("get URC-Port");
    if(!sendData("AT+QURCCFG?",1))
        return _URC_UNKNOW;

	while (1)
	{   
        if(serial_port->canReadLine())
        {
            String _str = serial_port->readLine();
            //debug("read data >> " +_str);
            if (_str.indexOf("usbat") != -1) {
                debug("URC : USB-AT");
                waitOK_ndb(_WAIT_OK_TIMEOUT);
                return _URC_USB_AT;
            }
            else if (_str.indexOf("usbmodem") != -1) {
                debug("URC : USB-MODEM");
                waitOK_ndb(_WAIT_OK_TIMEOUT);
                return _URC_USB_MODEM;
            }
            else if (_str.indexOf("uart1") != -1) {
                debug("URC : UART1");
                waitOK_ndb(_WAIT_OK_TIMEOUT);
                return _URC_UART1;
            }
        }
        else if(!serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("getURCPort >> Response timeout!!");
            return _URC_UNKNOW;
        }
	}
}

String HM_UC20CLASS::getIMEI(void)
{
    debug("get IMEI");
    if(!sendData("AT+CGSN",1))
        return "";

    while(1)
    {
        if(serial_port->canReadLine())
        {
            String _str = serial_port->readLine();
            //debug("read data >> " +_str);
            if (_str.length() > 5)
            {
                debug("IMEI : " + _str);
                waitOK(_WAIT_OK_TIMEOUT);
                return _str;
            }
        }
        else if(!serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("getIMEI >> Response timeout!!");
            return "";
        }
    }
}

String HM_UC20CLASS::getPhoneNum(OPERATOR_t _op)		//CUSD=1, //"*545*9#"-AIS, "*102*9#"-Dtac, "*933*9#"-True
{
    debug("get Phone number");
    if(_op == _OP_DTAC){
        if(!sendData("AT+CUSD=1,\"*102*9#\"",1))
            return "";
    }
    else if(_op == _OP_TRUE){
        if(!sendData("AT+CUSD=1,\"*933*9#\"",1))
            return "";
    }
    else if(_op == _OP_AIS){
        if(!sendData("AT+CUSD=1,\"*545*9#\"",1))
            return "";
    }

	while (1)
	{
        if(serial_port->canReadLine())
        {
            String _str = serial_port->readLine();
            //debug("read data >> " +_str);
            if (_str.indexOf("+CUSD:") != -1)
            {
                uint8_t _st = _str.indexOf("\"") + 1;
                String _res = _str.mid(_st,(uint8_t)_PHONE_NUMERAL_NO);
                debug("Phone Number : " + _res);
                clrSerialBuffer();
                return (_res);
            }
        }
        else if(!serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT + 10000)){
            debug("getPhoneNum >> Response timeout!!");
            return 0;
        }
	}
}

//private slot
void HM_UC20CLASS::slotReadyRead(void)
{
    QString _read_buf;

//    if(serial_port->canReadLine()){
//        _read_buf = serial_port->readLine();
//        if(_read_buf.indexOf("OK") != -1)
//            debug("#A");
//        debug("read data >> " + _read_buf);
//    }
    qDebug() << ">> " << serial_port->readAll();

//    for (int i = 0; i < _read_buf.size(); i++) {
//        _str += QString("%1 ").arg((u_int8_t)_read_buf[i] ,0 ,10);
//    }
}
