#include "sm_gsm_module.h"

SM_GSM_MODULE::SM_GSM_MODULE(QObject *parent) : QObject(parent)
{
#ifdef _GSM_MODULE_DEBUG
    logDebug = new SM_DEBUGCLASS("GSM-Module");
#endif
    connect(internet,SIGNAL(signalResetGsmModule()),this,SLOT(slotResetGsmModule()));
    connect(internet,SIGNAL(signalInitGsmModule()),this,SLOT(slotInit()));
    connect(this,SIGNAL(signalPacketNetworkIsRegis()),this,SLOT(slotSetInternet()));
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
    delete ftp;
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


//public//

//slot
void SM_GSM_MODULE::slotResetGsmModule()
{
    debug("Reset GsmModule");
//    if(flag_gsm_module_cannot_use)
//        return;

    for(int i=0; i < _TRY_TO_RESET_GSM_TIME; i++)
    {
        if(module->setPwrKey(_LOW))
        {
            if(slotInit())
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

bool SM_GSM_MODULE::slotInit(void)
{
    if (!module->isBegin()){
        if(!module->begin(9600)){
            return 0;
        }
    }

#ifdef Q_OS_LINUX
    if (!module->setPwrKey(_HIGH)){
        debug("Set power - Fail");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (!module->waitToReady(_WAIT_MODULE_RAEDY_TIME)) {
        debug("Wait to gsm ready : Timeout");
        if(!module->setURCPort(_URC_UART1)){
            debug("Can't set URC Port !!");
        }
        else{
//            debug("Set URC Port to UART1");
        }
        module->saveConfig();
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }
#endif

    if (!module->setEcho(_LOW)){
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (module->getURCPort() == _URC_UART1)
        debug("URC Port - OK");
    else{
        if(!module->setURCPort(_URC_UART1)){
            debug("Can't set URC Port !!");
            //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
            return 0;
        }
    }

    if (!sim->getSIMState()){
        debug("Sim - Not Ready!!");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (sim->getCCID().size() == 0){
        debug("CCID- Error!!");
        return 0;
    }

    if (!network->setOperator()){
        debug("Set Operator - Error!!");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (!network->getNetworkRegis()){
        debug("Set NetworkRegis - Error!!");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (network->getOperator() == ""){
        debug("Set getOperator - Error!!");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (network->getSignalQuality() == 101){
        debug("getSignalQuality = 101");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (!module->setPhoneFunc(4)){
        debug("setPhoneFunc(4) - Error!!");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    if (!module->setPhoneFunc(1)){
        debug("setPhoneFunc(1) - Error!!");
        //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
        return 0;
    }

    flag_packet_network_is_regis = false;
    packet_network_regis_timer.start();
    slotSetPacketNetworkRegis();

//    _timer.start();
//    while (!packet->getNetworkRegis()) { //wait to packet network ready
//        SM_DELAY::delay_ms(1000);
//        if(_timer.elapsed() > 20000){
//            debug("getNetworkRegis - Timeout!!");
//            return 0;
//        }
//    }
//    debug("wait time = " + String::number(_timer.elapsed()) + " [ms]");
//    emit signalGetNetworkRegisOK();
    return 1;
}

void SM_GSM_MODULE::slotSetPacketNetworkRegis()
{
    if(!flag_packet_network_is_regis)
    {
        if(!packet->getPacketNetworkRegis()){
            if(packet_network_regis_try_cnt < 20){
                QTimer::singleShot(2000,this,SLOT(slotSetPacketNetworkRegis())); //try again
                packet_network_regis_try_cnt++;
            }
            else{
                packet_network_regis_try_cnt = 0;
                debug("Packet network regis. - Timeout!!");
                QTimer::singleShot(10000,this,SLOT(slotResetGsmModule()));
            }
        }
        else{
            flag_packet_network_is_regis = true;
            debug("Packet network regis. - wait time = " + String::number(packet_network_regis_timer.elapsed()) + " [ms]");
            emit signalPacketNetworkIsRegis();
        }
    }

//    _timer.start();
//    while (!packet->getNetworkRegis()) { //wait to packet network ready
//        SM_DELAY::delay_ms(1000);
//        if(_timer.elapsed() > 20000){
//            debug("getNetworkRegis - Timeout!!");
//            return 0;
//        }
//    }
//    debug("wait time = " + String::number(_timer.elapsed()) + " [ms]");
//    emit signalGetNetworkRegisOK();
}

void SM_GSM_MODULE::slotSetInternet(void)
{
    if(internet->isConnect()){
        if(!internet->disConnect()){
            QTimer::singleShot(2000,this,SLOT(slotSetInternet())); //try again
            return;
        }
    }

    if (!http->setContextid(1)){
        QTimer::singleShot(2000,this,SLOT(slotSetInternet())); //try again
        return;
    }
    //ethernet.http.setResponseheader(0);
    if (!internet->configure()){
        QTimer::singleShot(2000,this,SLOT(slotSetInternet())); //try again
        return;
    }

    module_is_ready = true;
    emit signalInternetIsOK();
    emit signalSetLEDGsm(_LED_ON);
}
