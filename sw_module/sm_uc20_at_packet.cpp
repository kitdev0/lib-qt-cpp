#include "sm_uc20_at_packet.h"

SM_UC20_AT_PACKET_CLASS::SM_UC20_AT_PACKET_CLASS(HM_UC20CLASS* _module)
{
#ifdef _UC20_PACKET_DEBUG
	logDebug = new SM_DEBUGCLASS("UC20_PACKET");
#endif
    gsmModule = _module;
}

SM_UC20_AT_PACKET_CLASS::~SM_UC20_AT_PACKET_CLASS()
{
#ifdef _UC20_PACKET_DEBUG
	delete logDebug;
#endif
	delete gsmModule;
}

void SM_UC20_AT_PACKET_CLASS::debug(String data)
{
#ifdef _UC20_PACKET_DEBUG
#if _UC20_PACKET_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _UC20_PACKET_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _UC20_PACKET_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

bool SM_UC20_AT_PACKET_CLASS::getNetworkRegis(void)	//CGREG?
{
	uint8_t _state = 0;

    debug("get Network-Regis-Status - Packet");

    if(!gsmModule->sendData("AT+CGREG?",1))
        return 0;

	while (1)
	{       
        if(gsmModule->serial_port->canReadLine())
        {
            String _str = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_str.indexOf("+CGREG:") != -1)
            {
                int _index1 = _str.indexOf(",");
                int _index2 = _str.indexOf("\n");
                _state = _str.mid( _index1 + 1, _index2 - _index1).toInt();
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
            debug("getNetworkRegis [ packet ] >> Response timeout!!");
            return 0;
        }
	}
}
