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

    for(int i=0; i < _TRY_TO_RESET_GSM_TIME; i++){
//        if(module->setPwrKey(_LOW)){
//            break;
//        }
        if (!module->setPwrKey(_HIGH)){
            debug("Set power - Fail");
            //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
//            return 0;
        }

        if (!module->waitToReady(_WAIT_MODULE_RAEDY_TIME)) {
            debug("Wait to gsm ready : Timeout");
            emit signalTimeout();
            if(!module->setURCPort(_URC_UART1)){
                debug("Can't set URC Port !!");
            }
            else{
    //            debug("Set URC Port to UART1");
            }
            module->saveConfig();
            //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
//            return 0;
        }
        else{
            break;
        }
    }

    slotInit();
}

void SM_GSM_MODULE::slotResetNetwork()
{
    if(internet->resetConnecting()){
        return;
    }

    slotInit();
}

void SM_GSM_MODULE::slotInit(void)
{
    for(int i = 0 ; i < 10; i++){
        if(initModule()){
            flag_gsm_module_cannot_use = false;
            emit signalSetLEDGsm(_LED_ON);
            return;
        }
    }

    emit signalSetLEDGsm(_LED_OFF);
    flag_gsm_module_cannot_use = true;

    slotResetGsmModule();
}

bool SM_GSM_MODULE::slotBegin()
{
    if (!module->isBegin()){
        if(!module->begin(_BAUD_RATE_DEFINE)){
            return 0;
        }
    }

    if(module->getBaudRate() != _BAUD_RATE_DEFINE){

        module->closeSerial();
        module->begin(9600);

        module->setBaudRate(_BAUD_RATE_DEFINE);

        module->closeSerial();
        module->begin(_BAUD_RATE_DEFINE);

        module->getBaudRate();

        module->saveConfig();
    }

    slotResetGsmModule();
    return 1;
}

bool SM_GSM_MODULE::initModule(void)
{
    uint8_t _cnt = 0;

    debug("initModule");

    //read space
//    debug("Space UFS = " + String::number(file->getFreeSpace(_UFS),10));
//    debug("Space RAM = " + String::number(file->getFreeSpace(_RAM),10));

    if (!module->setEcho(_LOW)){
        return 0;
    }

    if (module->getURCPort() != _URC_UART1){
        if(!module->setURCPort(_URC_UART1)){
            debug("Can't set URC Port !!");
            return 0;
        }
    }

//    debug("#1");
    if (!sim->getSIMState()){
        debug("Sim - Not Ready!!");
        return 0;
    }

//    debug("#2");
    if (sim->getCCID().size() == 0){
        debug("CCID- Error!!");
        return 0;
    }

//    debug("#3");
    if (!network->setOperator()){
        debug("Set Operator - Error!!");
        return 0;
    }

//    debug("#4");
    if (!network->getNetworkRegis()){
        debug("Set NetworkRegis - Error!!");
        return 0;
    }

//    debug("#5");
    if (network->getOperator() == ""){
        debug("Set getOperator - Error!!");
        return 0;
    }

//    debug("#6");
    if (network->getSignalQuality() == 101){
        debug("getSignalQuality = 101");
        return 0;
    }

//    debug("#7");
    if (!module->setPhoneFunc(4)){
        debug("setPhoneFunc(4) - Error!!");
        return 0;
    }

//    debug("#8");
    if (!module->setPhoneFunc(1)){
        debug("setPhoneFunc(1) - Error!!");
        return 0;
    }

//    debug("#9");
//    flag_packet_network_is_regis = false;
//    slotSetPacketNetworkRegis();

    while (!packet->getPacketNetworkRegis()) {
        SM_DELAY::delay_ms(1000);
        _cnt++;
        if(_cnt > 60){
            debug("Packet network regis. - Timeout!!");
            return 0;
        }
    }

    if(internet->isConnect()){
        if(!internet->disConnect()){
            debug("Set to disable internet - Timeout!!");
            return 0;
        }
    }

    if (!http->setContextid(1)){
        debug("setContextid - Timeout!!");
        return 0;
    }

    if (!internet->configure()){
        debug("configure - Timeout!!");
        return 0;
    }

    module_is_ready = true;
    emit signalInternetIsOK();
    emit signalSetLEDGsm(_LED_ON);

    return 1;
}

void SM_GSM_MODULE::slotSetPacketNetworkRegis()
{
    if(!flag_packet_network_is_regis)
    {
        if(!packet->getPacketNetworkRegis()){
            if(packet_network_regis_try_cnt < 50){
//                debug("packet_network_regis_try_cnt = " + String::number(packet_network_regis_try_cnt));
                QTimer::singleShot(2000,this,SLOT(slotSetPacketNetworkRegis())); //try again
                packet_network_regis_try_cnt++;
            }
            else{
                packet_network_regis_try_cnt = 0;
                debug("Packet network regis. - Timeout!!");
                emit signalTimeout();
                debug("@12");
                if(packet_network_regis_timeout_cnt < 10){
                    QTimer::singleShot(5000,this,SLOT(slotInit_All()));
                    packet_network_regis_timeout_cnt++;
                }
            }
        }
        else{
            packet_network_regis_try_cnt = 0;
            packet_network_regis_timeout_cnt = 0;
            flag_packet_network_is_regis = true;
//            debug("Packet network regis. - wait time = " + String::number(packet_network_regis_timer.elapsed()) + " [ms]");
            emit signalPacketNetworkIsRegis();
        }
    }
}

void SM_GSM_MODULE::slotSetInternet(void)
{
    debug("#A");
    if(internet->isConnect()){
        if(!internet->disConnect()){
            QTimer::singleShot(2000,this,SLOT(slotSetInternet())); //try again
            return;
        }
    }

    debug("#B");
    if (!http->setContextid(1)){
        QTimer::singleShot(2000,this,SLOT(slotSetInternet())); //try again
        return;
    }
    //etherne t.http.setResponseheader(0);

    debug("#C");
    if (!internet->configure()){
        QTimer::singleShot(2000,this,SLOT(slotSetInternet())); //try again
        return;
    }

    module_is_ready = true;
    emit signalInternetIsOK();
    emit signalSetLEDGsm(_LED_ON);
}
