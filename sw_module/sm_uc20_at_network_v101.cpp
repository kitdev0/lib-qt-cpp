#include "sm_uc20_at_network_v101.h"

SM_UC20_AT_NETWORK_CLASS::SM_UC20_AT_NETWORK_CLASS(HM_UC20CLASS* _module)
{
#ifdef _UC20_NETWORK_DEBUG
	logDebug = new SM_DEBUGCLASS("UC20_NETWORK");
#endif
    gsmModule = _module;
}

SM_UC20_AT_NETWORK_CLASS::~SM_UC20_AT_NETWORK_CLASS()
{
#ifdef _UC20_NETWORK_DEBUG
	delete logDebug;
#endif
	delete gsmModule;
}

void SM_UC20_AT_NETWORK_CLASS::debug(String data)
{
#ifdef _UC20_NETWORK_DEBUG
#if _UC20_NETWORK_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _UC20_NETWORK_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _UC20_NETWORK_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

bool SM_UC20_AT_NETWORK_CLASS::getNetworkRegis(void)	//CREG?
{
	uint8_t _state = 0;

    debug("get Network-Regis.");

    if(!gsmModule->sendData("AT+CREG?",1))
        return 0;

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("+CREG:") != -1)
            {
                int _index1 = _str.indexOf(",");
                int _index2 = _str.indexOf("\n");
                _state = _str.mid(_index1 + 1, _index2 - _index1).toInt();
                debug(_str);
                if (_state == 1) {
                    debug("<< Ready");
                    return (gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT));
                }
                else {
                    debug("<< Not ready!!");
                    gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT);
                    return 0;
                }
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return 0;
        }
	}
}

uint8_t SM_UC20_AT_NETWORK_CLASS::getSignalQuality(void)
{
    uint8_t _signal = 101;

    debug("get Signal-Quality");

    if(!gsmModule->sendData("AT+CSQ",1))
        return 0;

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
//          debug("read data >> " + _str);
            if (_str.indexOf("+CSQ:") != -1)
            {
                int _index1 = _str.indexOf(" ");
                int _index2 = _str.indexOf(",");
                _signal = _str.mid(_index1 + 1, _index2 - _index1 - 1).toInt();
                debug("<< " + String::number(_signal,10));
                gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT);
                return(_signal);
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return _signal;
        }
	}
}

String SM_UC20_AT_NETWORK_CLASS::getOperator(void)
{
    debug("get Operator");

    if(!gsmModule->sendData("AT+COPS?",1))
        return 0;

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("+COPS:") != -1)
            {
                //+COPS: 0,0,"TRUE-H",2
                /*
                char comma1 = req.indexOf(F(","));
                char comma2 = req.indexOf(F(","),comma1+1);
                char comma3 = req.indexOf(F(","),comma2+1);
                String  operate_name = req.substring(comma2+2,comma3-1);
                String  acc_tech = req.substring(comma3+1);
                return(operate_name + "," + acc_tech);
                */
                String _res = _str.mid(_str.indexOf("\""));
                //debug("Operator =  " + _str);
                debug("<< " + _res);
                gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT);
                return(_res);
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return "";
        }

	}
}

bool SM_UC20_AT_NETWORK_CLASS::setOperator(void)
{
	String _str = "AT+COPS=1,0,";
    String _debug = "set Operator >> ";
#ifdef _OPERATOR
#if _OPERATOR == _DTAC
		_str += "\"TH-DTAC\",0";
#elif _OPERATOR == _TRUE
		_str += "\"TRUE\",0";
#elif _OPERATOR == _AIS
		_str += "\"TH GSM\",0";
#endif // 
#endif //
	debug(_debug + _str);
    if(!gsmModule->sendData(_str,1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT + 15000));
}



