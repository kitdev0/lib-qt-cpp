#include "sm_uc20_at_http.h"

bool flag_connect = false;

SM_UC20_AT_HTTP_CLASS::SM_UC20_AT_HTTP_CLASS(HM_UC20CLASS* _module)
{
#ifdef _UC20_HTTP_DEBUG
	logDebug = new SM_DEBUGCLASS("UC20_HTTP");
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
    debug(_debug + String::number(_context_ID,10));

    if(!gsmModule->sendData(_data + String::number(_context_ID,10),1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_HTTP_CLASS::setRequestHeader(bool _value)
{
    String _data = "AT+QHTTPCFG=\"requestheader\",";
    String _debug = "set Request-Header >> ";
	if (_value) {
		debug(_debug + "1");
        if(!gsmModule->sendData(_data + "1" ,1))
            return 0;
	}
	else {
		debug(_debug + "0");
        if(!gsmModule->sendData(_data + "0",1))
            return 0;
	}

    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
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

	return(gsmModule->waitOK(_WAIT_OK_TIMEOUT + 5000));
}

bool SM_UC20_AT_HTTP_CLASS::setURL(String _url)
{
    String _data = "AT+QHTTPURL=";
    String _debug = "setURL - utl >> ";
//	debug(_debug + _url);

    _data += String::number(_url.length(),10);
    _data += ",80";

//  debug("#set url >> " + _data);

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
                debug("<< " + _str);
                return 0;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("setURL-1 >> Response timeout!!");
            return 0;
        }
	}

    if(!gsmModule->sendData(_url,1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));

}

bool SM_UC20_AT_HTTP_CLASS::setURL(String _url, bool _wait_flag)
{
    String _data = "AT+QHTTPURL=";
    String _debug = "setURL - utl >> ";
//	debug(_debug + _url);
	URL_BUFF = _url;

    _data += String::number(_url.length(),10);
    _data += ",80";

//  debug("#set url >> " + _data);

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
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("setURL-2 >> Response timeout!!");
            return 0;
        }
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
		return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
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

int16_t SM_UC20_AT_HTTP_CLASS::getMethod(bool _wait_flag)
{
//    debug("getMethod");
    if(!gsmModule->sendData("AT+QHTTPGET=80",1))
        return 0;
	flag_get_method = true;

	while (_wait_flag)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _data = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_data.indexOf("+QHTTPGET:") != -1)
            {
                char index1 = _data.indexOf(",");
                return(_data.mid(index1 + 1, 4).toInt());
            }
            else if (_data.indexOf("ERROR") != -1)
            {
                debug("ERROR!!");
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT + 5000)){
            debug("getMethod >> Response timeout!!");
            return -2;
        }
	}
	return 0;
}

String SM_UC20_AT_HTTP_CLASS::readMethod(bool _wait_flag)
{
	bool _flag_connect = false;
//    debug("readMethod");

    if(!gsmModule->sendData("AT+QHTTPREAD=80",1))
        return 0;

	flag_read_method = true;

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
                debug("readAPI-ERROR!!");
                return "";
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("readMethod >> Response timeout!!");
            return "";
        }
	}
	return "";
}

int16_t SM_UC20_AT_HTTP_CLASS::postMethod()
{
	return(postMethod(" "));
}

int16_t SM_UC20_AT_HTTP_CLASS::postMethod(String _data)
{
	String _post = "";
    String _debug = "post >> ";

//	debug(_debug + _data);

    _post += "AT+QHTTPPOST=";
    _post += String::number(_data.length(),10);
    _post += ",80,80";

//    debug("#set port >> " + _post);

    if(!gsmModule->sendData(_post,1))
        return 0;

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
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("postMethod-1 >> Response timeout!!");
            return -2;
        }
	}
	
    if(!gsmModule->sendData(_data,1))
        return 0;

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("+QHTTPPOST") != -1)
            {
                char index1 = _str.indexOf(",");
                //debug(" >> " + _str);
                return(_str.mid(index1 + 1, 3).toInt());
            }
            else if (_str.indexOf("ERROR") != -1)
            {
                debug("ERROR2 : " + _str);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT)){
            debug("postMethod-2 >> Response timeout!!");
            return -2;
        }
	}

}

void SM_UC20_AT_HTTP_CLASS::timeoutStart(void)
{
	gsmModule->timeoutStart();
}


