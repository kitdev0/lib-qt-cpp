#include "sm_gsm_module.h"

SM_GSM_MODULE::SM_GSM_MODULE(QObject *parent) : QObject(parent)
{
#ifdef _GSM_MODULE_DEBUG
    logDebug = new SM_DEBUGCLASS("GSM-Module");
#endif
    connect(internet,SIGNAL(signalResetGsmModule()),this,SLOT(slotResetGsmModule()));
}

SM_GSM_MODULE::~SM_GSM_MODULE()
{
#ifdef _GSM_MODULE_DEBUG
    delete logDebug;
#endif
    delete module;
    delete sim;
    delete network;
    delete packet;
    delete internet;
    delete http;
    delete file;
}

void SM_GSM_MODULE::debug(String data)
{
#ifdef _GSM_MODULE_DEBUG
#if _GSM_MODULE_DEBUG == _DEBUG_SAY_ONLY
    logDebug->say(data);
#elif _GSM_MODULE_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _GSM_MODULE_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}
//private//


//-------- public -------//

//bool SM_GSM_MODULE::waitToReady(uint32_t _wait_time)
//{
//	//timeoutStart();
//	if(!ethernet.module.is_begin)
//		ethernet.module.begin(9600,GSM_PWRKEY);
//	if (ethernet.module.setPwr(HIGH)) {
//		if (!ethernet.module.waitToReady(_wait_time)) {
//			debug(F("Wait to gsm ready : Timeout"));
//			gsm_is_connect = false;
//			return false;
//		}
//	}
//	else {
//		debug(F("Wait to gsm ready : Timeout"));
//		gsm_is_connect = false;
//		return false;
//	}

//	debug("Wait to gsm ready = " + String(millis() - previous_time) + " [ms]");
//	return true;
//}

bool SM_GSM_MODULE::init(void)
{
    QElapsedTimer _timer;

    if (!module->isBegin())
        if(!module->begin(9600)) return 0;

#ifdef Q_OS_LINUX
    if (!module->setPwr(_HIGH)){
        debug("Set power - Fail");
        return 0;
    }

    if (!module->waitToReady(_WAIT_MODULE_RAEDY_TIME)) {
        debug("Wait to gsm ready : Timeout");
        return 0;
    }
#endif

    if (!module->setEcho(_LOW)) return 0;

    if (module->getURCPort() == _URC_UART1)
        debug("URC Port - OK");
    else{
        if(!module->setURCPort(_URC_UART1)){
            debug("Can't set URC Port !!");
            return 0;
        }
    }

    if (!sim->getSIMState()) return 0;

    if (!network->setOperator()) return 0;

    if (!network->getNetworkRegis()) return 0;

    if (network->getOperator() == "") return 0;

    if (!network->getSignalQuality()) return 0;

    if (!module->setPhoneFunc(4)) return 0;

    if (!module->setPhoneFunc(1)) return 0;

    _timer.start();
    while (!packet->getNetworkRegis()) { //wait to packet network ready
        SM_DELAY::delay_ms(1000);
        if(_timer.elapsed() > 20000){
            debug("getNetworkRegis - Timeout!!");
            return 0;
        }
    }
    debug("wait time = " + String::number(_timer.elapsed()) + " [ms]");
    emit signalGetNetworkRegisOK();
    return 1;
}

bool SM_GSM_MODULE::setInternet(void)
{
    if (!internet->disConnect())
        return 0;
    if (!http->setContextid(1))
        return 0;
    //ethernet.http.setResponseheader(0);
    if (!internet->configure())
        return 0;

    module_is_ready = true;
    emit signalInternetIsOK();
    emit signalSetLEDGsm(_LED_ON);
    return 1;
}
//public//

//slot
void SM_GSM_MODULE::slotResetGsmModule()
{
    debug("Reset GsmModule");
//    if(flag_gsm_module_cannot_use)
//        return;

    for(int i=0; i < _TRY_TO_RESET_GSM_TIME; i++)
    {
        if(module->setPwr(_LOW))
        {
            if(init())
            {
                flag_gsm_module_cannot_use = false;
                emit signalSetLEDGsm(_LED_ON);
                return;
            }
        }
    }

    emit signalSetLEDGsm(_LED_OFF);
    flag_gsm_module_cannot_use = true;
}
