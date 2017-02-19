#include "sm_uc20_at_sim.h"

SM_UC20_AT_SIM_CLASS::SM_UC20_AT_SIM_CLASS(HM_UC20CLASS* _module)
{
#ifdef _UC20_SIM_DEBUG
	logDebug = new SM_DEBUGCLASS("UC20_SIM");
#endif
    gsmModule = _module;
}

SM_UC20_AT_SIM_CLASS::~SM_UC20_AT_SIM_CLASS()
{
#ifdef _UC20_SIM_DEBUG
	delete logDebug;
#endif
    delete gsmModule;
}

void SM_UC20_AT_SIM_CLASS::debug(String data)
{
#ifdef _UC20_SIM_DEBUG
#if _UC20_SIM_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _UC20_SIM_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _UC20_SIM_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

//public

//bool SM_UC20_AT_SIM_CLASS::getSIMState(void)
//{
//	debug("Hello");
//}

bool SM_UC20_AT_SIM_CLASS::getSIMState(void)	//CPIN?
{
    debug("get SIM-state");

    if(!gsmModule->sendData("AT+CPIN?",1))
        return 0;

	while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("READY") != -1)
            {
                debug("<< READY");
                return(gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT));
            }
            if (_str.indexOf("ERROR") != -1)
            {
                debug("<< " + _str);
                return 0;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("getSIMState >> Response timeout!!");
            return 0;
        }
	}
}

String SM_UC20_AT_SIM_CLASS::getCCID(void)				//QCCID
{
    debug("get CCID");

    if(!gsmModule->sendData("AT+QCCID",1))
        return 0;

	while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
//            debug("read data >> " + _str);
            if (_str.indexOf("+QCCID:") != -1)
            {
                int _index1 = _str.indexOf(" ") + 1;
                int _index2 = _str.length() - _index1 - 2;
//                debug("#A = " + String::number(_index2));
                String _res = _str.mid(_index1,_index2);
                debug("<< " + _res);
                gsmModule->waitOK_ndb(_WAIT_OK_TIMEOUT);
                return (_res);
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("getCCID >> Response timeout!!");
            return "";
        }
	}
}
