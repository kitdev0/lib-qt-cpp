#include "sm_uc20_at_internet.h"

SM_UC20_AT_INTERNET_CLASS::SM_UC20_AT_INTERNET_CLASS(HM_UC20CLASS* _module, QObject *parent):
    QObject(parent)
{
#ifdef _UC20_INTERNET_DEBUG
    logDebug = new SM_DEBUGCLASS("UC15_INTERNET");
#endif
    gsmModule = _module;
}

SM_UC20_AT_INTERNET_CLASS::~SM_UC20_AT_INTERNET_CLASS()
{
#ifdef _UC20_INTERNET_DEBUG
	delete logDebug;
#endif
	delete gsmModule;
}

void SM_UC20_AT_INTERNET_CLASS::debug(String data)
{
#ifdef _UC20_INTERNET_DEBUG
#if _UC20_INTERNET_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _UC20_INTERNET_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _UC20_INTERNET_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

bool SM_UC20_AT_INTERNET_CLASS::configure(String _apn, String _user, String _password)
{
	String _str = "AT+QICSGP=1,1,";
    String _debug = "configure-APN >> ";
	debug(_debug + "APN:" + _apn + " USER:" + _user + " PASS:" + _password);
	_str += "\"" + _apn + "\",";
	_str += "\"" + _user + "\",";
	_str += "\"" + _password + "\",";
	_str += "1";
    if(!gsmModule->sendData(_str,1))
        return 0;
	return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_INTERNET_CLASS::configure(void)
{
	String _str = "AT+QICSGP=1,1,";
    String _debug = "configure-APN >> ";
//#ifdef _OPERATOR
//#if _OPERATOR == _DTAC
//	_str += "\"www.dtac.co.th\",";
//	_str += "\"\",";
//	_str += "\"\",";
//	_str += "1";
//#elif _OPERATOR == _TRUE
//	_str += "\"internet\",";
//	_str += "\"True\",";
//	_str += "\"true\",";
//	_str += "1";
//#elif _OPERATOR == _AIS
//	_str += "\"internet\",";
//	_str += "\"\",";
//	_str += "\"\",";
//	_str += "1";
//#endif //
//#endif //
    _str += "\"" + String(_APN) + "\",";
    _str += "\"" + String(_USER) + "\",";
    _str += "\"" + String(_PASS) + "\",";
    _str += "1";
	debug(_debug + _str);
    if(!gsmModule->sendData(_str,1))
        return 0;
	return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_INTERNET_CLASS::connect(void)
{
    QElapsedTimer _timer;
//    debug("Set to open internet");
    if(!gsmModule->sendData("AT+QIACT=1",1)){
        debug("Set Connect internet >> Faile");
        return 0;
    }
    _timer.start();
    if (gsmModule->waitOK(_WAIT_OK_TIMEOUT+5000)){
        debug("Set Connect internet >> OK");
        debug("wait time : " + String::number(_timer.elapsed()));
        return 1;
	}
    else{
        _timer.start();
        if (gsmModule->waitOK(_WAIT_OK_TIMEOUT+5000)){
            debug("Set Connect internet >> OK");
            debug("wait time : " + String::number(_timer.elapsed()));
            return 1;
        }
        debug("Set Connect internet >> Faile!!");
//        emit signalResetGsmModule();
        return 0;
    }
}

bool SM_UC20_AT_INTERNET_CLASS::disConnect()
{
//    debug("Set - Disable Internet");
    if(!gsmModule->sendData("AT+QIDEACT=1",1)){
        debug("Set Disable internet >> Faile");
        return 0;
    }

    if(gsmModule->waitOK(_WAIT_OK_TIMEOUT+5000)){
        debug("Set Disable internet >> Success");
        return 1;
    }
    else{
        debug("Set Disable internet >> Faile");
        return 0;
    }
}

bool SM_UC20_AT_INTERNET_CLASS::isConnect(void)
{
//    debug("Check Connecting");
    if(getIP().length()){
//        debug("CONNECTING...");
        return 1;
    }
    else{
//        debug("DISCONNECT");
        return 0;
    }
//    if(!gsmModule->sendData("AT+QIACT?",1))
//        return 0;

//	while (1)
//	{
//        if(gsmModule->serial_port->canReadLine())
//        {
//            String _str = gsmModule->serial_port->readLine();
//            //debug("read data >> " + _str);
//            if (_str.indexOf("+QIACT:") != -1)
//            {
//                gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT);
//                debug("CONNECTING...");
//                return 1;
//            }
//            else if (_str.indexOf("OK") != -1)
//            {
//                debug("DISCONNECT");
//                return 0;
//            }
//        }
//        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
//            debug("Response timeout!!");
//            return 0;
//        }
//	}


}

String SM_UC20_AT_INTERNET_CLASS::getIP()
{
//    debug("get-IP");
    if(!gsmModule->sendData("AT+QIACT?",1))
        return 0;
	gsmModule->clrSerialBuffer();

	while (1)
	{    
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("+QIACT:") != -1)
            {
                char index1 = _str.indexOf("\"");
                char index2 = _str.indexOf(("\""), index1 + 1);
                gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT);
                return(_str.mid(index1 + 1, index2 - index1));
            }
            else if (_str.indexOf("OK") != -1)
            {
                debug("Check IP >> Internet disconnect!!");
                return "";
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Check IP >> Response timeout!!");
            return "";
        }
	}
}

bool SM_UC20_AT_INTERNET_CLASS::resetConnecting(void)
{
    debug("Reset Connecting");

    gsmModule->sendData("+++",1); // Exit from post method loob.

    for(int i=0;i < 5;i++){
        if(disConnect())
            return isConnect();
    }

    emit signalResetGsmModule();
    return false;
}
