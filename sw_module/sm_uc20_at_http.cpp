#include "sm_uc20_at_http.h"

bool flag_connect = false;

QElapsedTimer my_timer;

SM_UC20_AT_HTTP_CLASS::SM_UC20_AT_HTTP_CLASS(HM_UC20CLASS* _module)
{
#ifdef _UC20_HTTP_DEBUG
    logDebug = new SM_DEBUGCLASS("UC15_HTTP");
#endif
    gsmModule = _module;
}

SM_UC20_AT_HTTP_CLASS::~SM_UC20_AT_HTTP_CLASS()
{
#ifdef _UC20_HTTP_DEBUG
	delete logDebug;
#endif
	delete gsmModule;
}

void SM_UC20_AT_HTTP_CLASS::debug(String data)
{
#ifdef _UC20_HTTP_DEBUG
#if _UC20_HTTP_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _UC20_HTTP_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _UC20_HTTP_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

//public
//void SM_UC20_AT_HTTP_CLASS::read(void)
//{
//	debug(F("read"));
//	gsmModule->serial_port->println("AT+QHTTPREAD=80");
//	//+QHTTPREAD: 0	
//}

bool SM_UC20_AT_HTTP_CLASS::checkResponseTimeout(void)
{
	if (flag_set_url || flag_send_url || flag_get_method || flag_read_method) 
	{
        if (gsmModule->timeoutCheck(_WAIT_RESPONSE_TIMEOUT)){
			gsmModule->timeoutStart();
            debug("Timeout!!");
			return true;
		}
	}
	return false;
}

void SM_UC20_AT_HTTP_CLASS::readReturn(void)
{
    if (gsmModule->dataAvailable()) {
        char _data = gsmModule->receiveData();
		return_data += _data;
	}
	//if (flag_connect) {
	//	response_data = return_data;
	//	flag_connect = false;
	//}
	if (return_data.indexOf('\n') != -1)
	{
		//debug("readReturn = " + return_data);
		if (flag_set_url) {
            if (return_data.indexOf("CONNECT") != -1) {
				flag_ready_to_send_url = true;
				flag_set_url = false;
			}
		}
		if (flag_send_url) {
            if (return_data.indexOf("OK") != -1) {
				flag_ready_to_get_method = true;
				flag_send_url = false;
			}
		}
		if (flag_get_method) {
            if (return_data.indexOf("+QHTTPGET:") != -1)
			{
                char index1 = return_data.indexOf(",");
                int16_t _value =  return_data.mid(index1 + 1, 4).toInt();
				//debug("response value = " + String(_value));
				server_response_value = _value;
				flag_ready_to_read_method = true;
				flag_get_method = false;
			}
            if (return_data.indexOf("ERROR") != -1)
			{
				flag_get_method_error = true;
				flag_get_method = false;
			}
		}
		if (flag_read_method) {
            if (return_data.indexOf("CONNECT") != -1) {
				flag_read_method_connect = true;
			}
			if (flag_read_method_connect) {
				response_data = return_data;
				flag_read_method_connect = false;
			}
            if (return_data.indexOf("+QHTTPREAD") != -1) {
				flag_response_data_is_ok = true;
				flag_read_method = false;
			}
            if (return_data.indexOf("+CME ERROR :") != -1) {
                debug("readAPI-ERROR!!");
				flag_read_method_error = true;
				flag_read_method = false;
			}
		}
		return_data = "";
	}
	//else if (return_data.indexOf(F("+QHTTPREAD")) != -1) {
	//	String _str = response_data;
	//}
	//else if (return_data.indexOf(F("+CME ERROR :")) != -1) {
	//	debug(F("readAPI-ERROR!!"));
	//}
	//if (gsmModule->timeoutCheck(_WAIT_OK_TIMEOUT + 5000))
	//{
	//	debug(F("Timeout!!"));
	//}
}

void SM_UC20_AT_HTTP_CLASS::resetFlag(void)
{
	flag_set_url = false;
	flag_send_url = false;
	flag_get_method = false;
	flag_read_method = false;
	flag_ready_to_set_url = false;
	flag_ready_to_send_url = false;
	flag_ready_to_get_method = false;
	flag_ready_to_read_method = false;
	flag_get_method_error = false;
	flag_response_data_is_ok = false;
	flag_read_method_connect = false;
	flag_read_method_error = false;
	server_response_value = 0;
}

bool SM_UC20_AT_HTTP_CLASS::setContextid(int8_t _context_ID)
{
    String _data = "AT+QHTTPCFG=\"contextid\",";
    String _debug = "set Context-ID >> ";
//    debug(_debug + String::number(_context_ID,10));

    if(!gsmModule->sendData(_data + String::number(_context_ID,10),1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT, "setContextid"));
}

bool SM_UC20_AT_HTTP_CLASS::setRequestHeader(bool _value)
{
    String _data = "AT+QHTTPCFG=\"requestheader\",";
    String _debug = "set Request-Header >> ";
	if (_value) {
//		debug(_debug + "1");
        if(!gsmModule->sendData(_data + "1" ,1))
            return 0;
	}
	else {
//		debug(_debug + "0");
        if(!gsmModule->sendData(_data + "0",1))
            return 0;
	}

    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT, "setRequestHeader"));
}

bool SM_UC20_AT_HTTP_CLASS::setResponseHeader(bool _value)
{
    String _data = "AT+QHTTPCFG=\"responseheader\",";
    String _debug = "set Response-Header >> ";
	if (_value) {
		debug(_debug + "1");
        if(!gsmModule->sendData(_data + "1",1))
            return 0;
	}
	else {
		debug(_debug + "0");
        if(!gsmModule->sendData(_data + "0",1))
            return 0;
	}

    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT + 5000, "setResponseHeader"));
}

bool SM_UC20_AT_HTTP_CLASS::setURL(String _url)
{
    String _data = "AT+QHTTPURL=";
    String _debug = "setURL - utl >> ";
    uint8_t _cnt = 0;
//	debug(_debug + _url);

    _data += String::number(_url.length(),10);
    _data += ",";
    _data += String::number(_URL_INPUT_TIME,10);

//    debug("#set url >> " + _url);

    if(!gsmModule->sendData(_data,1))
        return 0;

	while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("CONNECT") != -1)
            {
//                debug("<< " + _str);
                break;
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _str);
                return 0;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_URL_INPUT_TIME + 10)){
                debug("setURL-1 >> Response timeout!!");
                return 0;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
//            debug("setURL-1 >> Response timeout!!");
//            return 0;
//        }
	}

    if(!gsmModule->sendData(_url,1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT, "setURL"));

}

bool SM_UC20_AT_HTTP_CLASS::setURL(String _url, bool _wait_flag)
{
    String _data = "AT+QHTTPURL=";
    String _debug = "setURL - utl >> ";
    uint8_t _cnt = 0;
//	debug(_debug + _url);
	URL_BUFF = _url;

    _data += String::number(_url.length(),10);
    _data += ",";
    _data += String::number(_URL_INPUT_TIME,10);

//    debug("#set url >> " + _url);

    if(!gsmModule->sendData(_data,1))
        return 0;

	flag_set_url = true;

	while (_wait_flag)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
//          debug("read data >> " + _str);
            if (_str.indexOf("CONNECT") != -1)
            {
//                debug("<< " + _str);
                return true;
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("<< " + _str);
                return false;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_URL_INPUT_TIME + 10)){
                debug("setURL-2 >> Response timeout!!");
                return 0;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
//            debug("setURL-2 >> Response timeout!!");
//            return 0;
//        }
	}
	return false;
}

bool SM_UC20_AT_HTTP_CLASS::sendURL(bool _wait_flag)
{
    debug("sendURL");
    if(!gsmModule->sendData(URL_BUFF,1))
        return 0;

	flag_send_url = true;

	if (_wait_flag) {
        return(gsmModule->waitOK(_WAIT_OK_TIMEOUT, "sendURL"));
	}
    return 0;
}

bool SM_UC20_AT_HTTP_CLASS::saveResponseToMemory(FILE_PATTERN_t pattern, String Filename)
{
    String _data = "";
	debug("SaveResponseToMemory");

	if (pattern == _UFS)
        _data += ("AT+QHTTPREADFILE=\"");
	if (pattern == _RAM)
        _data += ("AT+QHTTPREADFILE=\"RAM:");
	_data += Filename;
    _data += "\",80";

    if(!gsmModule->sendData(_data,1))
        return 0;

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("+QHTTPREADFILE: 0") != -1)
            {
                debug(" << Save file - OK");
                return 1;

            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug(" << Save file - Error!!");
                return 0;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("saveResponseToMemory >> Response timeout!!");
            return 0;
        }
	}

}

int16_t SM_UC20_AT_HTTP_CLASS::getMethodCustomHeader(String *_header, uint16_t _data_size, uint16_t _input_time_sec)
{
//    debug("getMethod");
    uint16_t _cnt = 0;
    String _get = "";

//	debug(_debug + _data);

    _get += "AT+QHTTPGET=";
    _get += String::number(_GET_RESPONSE_TIME);
    _get += ",";
    _get += String::number(_data_size);
    _get += ",";
    _get += String::number(_input_time_sec);


    if(!gsmModule->sendData(_get,1))
        return 0;

//    if(!gsmModule->sendData("AT+QHTTPGET=80",1))
//        return 0;

    flag_get_method = true;

//    myTimeoutReset();
//    debug("#D1");
    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _data = gsmModule->serial_port->readLine();
//            debug("read data >> " + _data);
            if (_data.indexOf("CONNECT") != -1)
            {
//                debug("CONNECTED");
                break;
            }
            else if (_data.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _data);
                debug("get Set >> " + _get);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_GET_RESPONSE_TIME + 10)){
                debug("getMethod-1 >> Response timeout!!");
                debug("get Set >> " + _get);
                return -2;
            }
        }
//        else if(myTimeoutCheck((_GET_RESPONSE_TIME * 1000) + 10000)){
//            debug("getMethod >> Response timeout!!");
//            return -2;
//        }
    }

//    debug("#D2");

    if(!gsmModule->sendData(*_header,1))
        return 0;

    _cnt = 0;

//    debug("#D3");

    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _data = gsmModule->serial_port->readLine();
//            debug("read data >> " + _data);
            if (_data.indexOf("+QHTTPGET:") != -1)
            {
                char index1 = _data.indexOf(",");
                return(_data.mid(index1 + 1, 4).toInt());
            }
            else if (_data.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _data);
                debug("get Set >> " + _get);
                debug("get Body >> " + *_header);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_input_time_sec + 10)){
                debug("getMethod-2 >> Response timeout!!");
                debug("get Set >> " + _get);
                debug("get Body >> " + *_header);
                return -2;
            }
        }
//        else if(myTimeoutCheck((_GET_RESPONSE_TIME * 1000) + 10000)){
//            debug("getMethod >> Response timeout!!");
//            return -2;
//        }
    }
    return 0;
}

int16_t SM_UC20_AT_HTTP_CLASS::getMethod(bool _wait_flag)
{
//    debug("getMethod");
    uint16_t _cnt = 0;
    String _get = "";

//	debug(_debug + _data);

    _get += "AT+QHTTPGET=";
    _get += String::number(_GET_RESPONSE_TIME);


    if(!gsmModule->sendData(_get,1))
        return 0;

//    if(!gsmModule->sendData("AT+QHTTPGET=80",1))
//        return 0;

	flag_get_method = true;

//    myTimeoutReset();

	while (_wait_flag)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _data = gsmModule->serial_port->readLine();
//            debug("read data >> " + _data);
            if (_data.indexOf("+QHTTPGET:") != -1)
            {
                char index1 = _data.indexOf(",");
                return(_data.mid(index1 + 1, 4).toInt());
            }
            else if (_data.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _data);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_GET_RESPONSE_TIME + 10)){
                debug("getMethod >> Response timeout!!");
                return -2;
            }
        }
//        else if(myTimeoutCheck((_GET_RESPONSE_TIME * 1000) + 10000)){
//            debug("getMethod >> Response timeout!!");
//            return -2;
//        }
	}
	return 0;
}

String SM_UC20_AT_HTTP_CLASS::readMethod(bool _wait_flag)
{
	bool _flag_connect = false;
    uint16_t _cnt = 0;
//    debug("readMethod");

    if(!gsmModule->sendData("AT+QHTTPREAD=80",1))
        return 0;

	flag_read_method = true;

//    myTimeoutReset();

	while (_wait_flag)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _data = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_flag_connect) {
                response_data = _data;
                _flag_connect = false;
            }
            if (_data.indexOf("CONNECT") != -1) {
                _flag_connect = 1;
            }
            else if (_data.indexOf("+QHTTPREAD") != -1) {
                return response_data;
            }
            else if (_data.indexOf("+CME ERROR :") != -1) {
                debug("readAPI-ERROR >> " + _data);
                return "-1";
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_READ_RESPONSE_TIME + 10)){
                debug("readMethod >> Response timeout!!");
                debug("Serial error = " + gsmModule->serial_port->errorString());
                gsmModule->serial_port->clear();
                gsmModule->serial_port->clearError();
                debug("Serial error after clear = " + gsmModule->serial_port->errorString());
                return "-2";
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
//            debug("readMethod >> Response timeout!!");
//            return "";
//        }
	}
    return "0";
}

int16_t SM_UC20_AT_HTTP_CLASS::postMethod()
{
    String _str = " ";
    return postMethod(&_str);
}

int16_t SM_UC20_AT_HTTP_CLASS::postMethod(String *_data)
{
	String _post = "";
//    String _debug = "post >> ";
    uint16_t _cnt = 0;

//	debug(_debug + _data);

    _post += "AT+QHTTPPOST=";
    _post += String::number(_data->length(),10);
    _post += ",";
    _post += String::number(_POST_INPUT_TIME);
    _post += ",";
    _post += String::number(_POST_RESPONSE_TIME);

//    debug("# data port >> ");
//    debug(*_data);

    if(!gsmModule->sendData(_post,1))
        return 0;

//    myTimeoutReset();

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("CONNECT") != -1)
            {
//                debug("CONNECTED");
                break;
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("ERROR1 : " + _str);
                debug("Post Set >> " + _post);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_POST_RESPONSE_TIME + 10)){
                debug("postMethod-1 >> Response timeout!!");
                debug("Post Set >> " + _post);
                return -2;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
//            debug("postMethod-1 >> Response timeout!!");
//            return -2;
//        }
	}
	
    if(!gsmModule->sendData(*_data,1))
        return 0;

//    myTimeoutReset();
    _cnt = 0;

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("+QHTTPPOST") != -1)
            {
                char index1 = _str.indexOf(",");
                debug(" >> " + _str);
                return(_str.mid(index1 + 1, 3).toInt());
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("ERROR2 : " + _str);
                debug("Post Set >> " + _post);
                debug("Post Body >> " + *_data);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_POST_RESPONSE_TIME + 10)){
                debug("postMethod-2  >> Response timeout!!");
                debug("Post Set >> " + _post);
                debug("Post Body >> " + *_data);
                return -2;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
//            debug("postMethod-2 >> Response timeout!!");
//            return -2;
//        }
	}
}

int16_t SM_UC20_AT_HTTP_CLASS::postMethod(QFile *_file, uint16_t _input_time, uint16_t _response_time)
{

    uint16_t _cnt = 0;
    if(!_file->exists()){
        debug("File not found!!");
        return 0;
    }
    if(!_file->open(QIODevice::ReadOnly)){
        debug("File can't open!!");
        return 0;
    }

    QByteArray _my_data;
    _my_data = _file->readAll();
//    _my_data = "Hi I'm Mr. Kiiitsak";

    _file->close();

    String _post = "";
    String _debug = "post >> ";

//	debug(_debug + _data);

    if(_my_data.length() > 1024000 || _my_data.length() == 0){
        _my_data = "!! Log-file over size !!";
    }

    _post += "AT+QHTTPPOST=";
    _post += String::number(_my_data.length(),10);
    _post += "," + String::number(_input_time,10);
    _post += "," + String::number(_response_time,10);
//    _post += ",80,80";

//    debug("#set port >> " + _post);

    if(!gsmModule->sendData(_post,1))
        return 0;

//    myTimeoutReset();

    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("CONNECT") != -1)
            {
//                debug("CONNECTED");
                break;
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("ERROR1 : " + _str);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_response_time + 10)){
                debug("#1 postLogFile  >> Response timeout!!");
                return -2;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_response_time + 30000)){
//            debug("#1 postLogFile >> Response timeout!!");
//            return -2;
//        }
    }

//    QByteArray _my_data;

//    while(!_file->atEnd()){
//        _my_data = _file->readAll();
    gsmModule->sendDataByte(_my_data,false);
//        gsmModule->sendDataByte("+++",true);
//    }

//    _file->close();
//    debug(_my_data);
//    timeout.start();

//    myTimeoutReset();
    _cnt = 0;

    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
//            debug("read data >> " + _str);
            if (_str.indexOf("+QHTTPPOST") != -1)
            {
                char index1 = _str.indexOf(",");
                return(_str.mid(index1 + 1, 3).toInt());
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("Post Log >> ERROR2 : " + _str);
                return -1;
            }
//            timeout.start();
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_response_time + 10)){
                debug("#2 postLogFile  >> Response timeout!!");
                return -2;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_response_time + 30000)){
//            debug("#2 postLogFile >> Response timeout!!");
//            return -2;
//        }

//        if(timeout.elapsed() > (180000)){
//            debug("#2 postLogFile >> Response timeout!!");
//            debug("timeout = " + String::number(timeout.elapsed()));
//            return -2;
//        }
    }
}

int16_t SM_UC20_AT_HTTP_CLASS::postMethodCustomHeader(String *_header, QFile *_file, uint16_t _input_time, uint16_t _response_time)
{

    uint16_t _cnt = 0;
    String _data = "";
    if(!_file->exists()){
        debug("File not found!!");
        return 0;
    }
    if(!_file->open(QIODevice::ReadOnly)){
        debug("File can't open!!");
        return 0;
    }

    _data = _file->readAll();
//    _my_data = "Hi I'm Mr. Kiiitsak";

    _file->close();

    String _post = "";
//    String _debug = "post >> ";

    if(_data.length() == 0){
        _data = "!! Log-file empty!!";
    }
    else if(_data.length() > _DATA_POST_MAX_SIZE){
        _data = "!! Log-file over size !!";
    }

    _data = *_header + _data;

    _post += "AT+QHTTPPOST=";
    _post += String::number(_data.length(),10);
    _post += "," + String::number(_input_time,10);
    _post += "," + String::number(_response_time,10);
//    _post += ",80,80";

//    debug("#set port >> " + _post);

    if(!gsmModule->sendData(_post,1))
        return 0;

//    myTimeoutReset();

    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("CONNECT") != -1)
            {
//                debug("CONNECTED");
                break;
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("Post Log >> ERROR1 : " + _str);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_response_time + 10)){
                debug("#1 postLogFile  >> Response timeout!!");
                return -2;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_response_time + 30000)){
//            debug("#1 postLogFile >> Response timeout!!");
//            return -2;
//        }
    }

//    QByteArray _my_data;

//    while(!_file->atEnd()){
//        _my_data = _file->readAll();
    gsmModule->sendData(_data,false);
//        gsmModule->sendDataByte("+++",true);
//    }

//    _file->close();
//    debug(_my_data);
//    timeout.start();

//    myTimeoutReset();
    _cnt = 0;

    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
//            debug("read data >> " + _str);
            if (_str.indexOf("+QHTTPPOST") != -1)
            {
                char index1 = _str.indexOf(",");
                return(_str.mid(index1 + 1, 3).toInt());
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("Post Log >> ERROR2 : " + _str);
                return -1;
            }
//            timeout.start();
        }
        else if(!gsmModule->serial_port->waitForReadyRead(1000)){
            _cnt++;
            if(_cnt > (_response_time + 10)){
                debug("#2 postLogFile  >> Response timeout!!");
                return -2;
            }
        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_response_time + 30000)){
//            debug("#2 postLogFile >> Response timeout!!");
//            return -2;
//        }

//        if(timeout.elapsed() > (180000)){
//            debug("#2 postLogFile >> Response timeout!!");
//            debug("timeout = " + String::number(timeout.elapsed()));
//            return -2;
//        }
    }
}

void SM_UC20_AT_HTTP_CLASS::timeoutStart(void)
{
	gsmModule->timeoutStart();
}

void SM_UC20_AT_HTTP_CLASS::myTimeoutReset(void)
{
    my_timer.start();
}

bool SM_UC20_AT_HTTP_CLASS::myTimeoutCheck(uint64_t _time)
{
    if (my_timer.elapsed() > _time)
    {
        return 1;
    }
    return 0;
}


