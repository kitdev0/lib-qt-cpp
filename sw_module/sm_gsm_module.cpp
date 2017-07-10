#include "sm_gsm_module.h"

SM_GSM_MODULE::SM_GSM_MODULE(QObject *parent) : QObject(parent)
{
#ifdef _GSM_MODULE_DEBUG
    logDebug = new SM_DEBUGCLASS("GSM-Module");
#endif
//    connect(internet,SIGNAL(signalResetGsmModule()),this,SLOT(slotResetGsmModule()));
//    connect(internet,SIGNAL(signalInitGsmModule()),this,SLOT(slotInit()));
    connect(this,SIGNAL(signalModuleIsBeginSuccess()),this,SLOT(slotInitModule()));
    connect(this,SIGNAL(signalModuleIsInitSuccess()),this,SLOT(slotSetPacketNetworkRegis()));
    connect(this,SIGNAL(signalPacketNetworkIsRegis()),this,SLOT(slotInitInternet()));
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
bool SM_GSM_MODULE::begin()
{
    internet_is_ready = false;

    if (!module->isBegin()){
        if(!module->begin(_BAUD_RATE_DEFINE)){
            return 0;
        }
    }

    if(module->getBaudRate() != _BAUD_RATE_DEFINE){
        debug("!! Warning >> Baudrate is incorrect");
        module->closeSerial();
        module->begin(9600);

        module->setBaudRate(_BAUD_RATE_DEFINE);
        debug("Set Baudrate to " + String::number(_BAUD_RATE_DEFINE,10));

        module->closeSerial();
        module->begin(_BAUD_RATE_DEFINE);

        module->saveConfig();

        if(module->getBaudRate() == _BAUD_RATE_DEFINE){
            debug("Baudrate Setting >> OK");
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
//        slotInit();
        debug("Baudrate Setting >> OK");
        return 1;
    }

//    slotResetGsmModule();
}

bool SM_GSM_MODULE::initModule(void)
{
    internet_is_ready = false;

    module->clrSerialBuffer();

    if (!module->setEcho(_LOW)){
        return 0;
    }

    if(!module->setFlowDTR_OFF()){
        return 0;
    }

    if (module->getURCPort() != _URC_UART1){
        if(!module->setURCPort(_URC_UART1)){
            debug("!! Warning : Can't set URC Port !!");
            return 0;
        }
        module->saveConfig();
    }

//    debug("#1");
    if (!sim->getSIMState()){
        debug("!! Warning : Sim - Not Ready!!");
        return 0;
    }

//    debug("#2");
    if (sim->getCCID().size() == 0){
        debug("!! Warning : CCID- Error!!");
        return 0;
    }

//    debug("#3");
    if (!network->setOperator()){
        debug("!! Warning : Set Operator - Error!!");
        return 0;
    }

//    debug("#4");
    if (!network->getNetworkRegis()){
        debug("!! Warning : Set NetworkRegis - Error!!");
        return 0;
    }

//    debug("#5");
    if (network->getOperator() == ""){
        debug("!! Warning : Set getOperator - Error!!");
        return 0;
    }

//    debug("#6");
    if (network->getSignalQuality() == 101){
        debug("!! Warning : getSignalQuality = 101");
        return 0;
    }

//    debug("#7");
    if (!module->setPhoneFunc(4)){
        debug("!! Warning : setPhoneFunc(4) - Error!!");
        return 0;
    }

//    debug("#8");
    if (!module->setPhoneFunc(1)){
        debug("!! Warning : setPhoneFunc(1) - Error!!");
        return 0;
    }

//    flag_gsm_module_cannot_use = false;
    emit signalSetLEDGsm(_LED_ON);

    return 1;
}

void SM_GSM_MODULE::setPwrOffModule()
{
    if(try_to_set_pwr_off < 2){
        try_to_set_pwr_off++;
        QTimer::singleShot(10000,this,SLOT(slotTryToSetPwrOFF()));
        return;
    }

    if(try_to_set_pwr_off < 5){
        try_to_set_pwr_off++;
        QTimer::singleShot(5*60000,this,SLOT(slotTryToSetPwrOFF()));
        return;
    }

    if(try_to_set_pwr_off < 10){
        try_to_set_pwr_off++;
        QTimer::singleShot(15*60000,this,SLOT(slotTryToSetPwrOFF()));
        return;
    }
    QTimer::singleShot(30*60000,this,SLOT(slotTryToSetPwrOFF()));
}

//slot
void SM_GSM_MODULE::slotTryToSetPwrOFF()
{
    debug("## Try to Set Power >> OFF : [" + String::number(try_to_set_pwr_unsuccess) + "]");
    module->clrSerialBuffer();

    module->setPwrOff();
    SM_DELAY::delay_ms(5000);
    module->setPwrOn();

    module->setPwrKey(_HIGH);

    SM_DELAY::delay_sec(10);

    if(!module->waitToReady(_WAIT_MODULE_RAEDY_TIME)){
        if(try_to_set_pwr_unsuccess < 5){
            try_to_set_pwr_unsuccess++;
            QTimer::singleShot(5000,this,SLOT(slotTryToSetPwrOFF()));
        }
        else{
            //add clear buffer 21/06/2017
            module->closeSerial();
            QTimer::singleShot(5000,this,SLOT(slotBegin()));
            try_to_set_pwr_unsuccess = 0;
//            QTimer::singleShot(15*60000,this,SLOT(slotTryToSetPwrOFF()));
        }
    }
    else{
        try_to_set_pwr_unsuccess = 0;
        QTimer::singleShot(10000,this,SLOT(slotBegin()));
    }

//    if(!module->setCmdPwrOff()){
//        if(try_to_set_pwr_unsuccess < 5){
//            try_to_set_pwr_unsuccess++;
//            QTimer::singleShot(5000,this,SLOT(slotTryToSetPwrOFF()));
//        }
//        else{
//            //add clear buffer 21/06/2017
//            module->closeSerial();
//            QTimer::singleShot(5000,this,SLOT(slotBegin()));
//            try_to_set_pwr_unsuccess = 0;
////            QTimer::singleShot(15*60000,this,SLOT(slotTryToSetPwrOFF()));
//        }
//    }
//    else{
//        try_to_set_pwr_unsuccess = 0;
//        QTimer::singleShot(10000,this,SLOT(slotBegin()));
//    }
}

void SM_GSM_MODULE::slotBegin()
{
    debug("## slotBegin [" + String::number(try_to_begin_module) +"]");

    if(begin()){
        try_to_begin_module = 0;
        emit signalModuleIsBeginSuccess();
    }
    else{
//        debug("begin_gsm_module_cnt++");
//        debug("try_to_begin_module = " + String::number(try_to_begin_module));
        try_to_begin_module++;
        if(try_to_begin_module > 5){
            try_to_begin_module = 0;
            setPwrOffModule();
        }
        else{
            QTimer::singleShot(10000,this,SLOT(slotBegin()));
        }
    }
}

void SM_GSM_MODULE::slotInitModule()
{
    debug("## slotInitModule [" + String::number(try_to_init_module) +"]");

    if(initModule()){
        flag_packet_network_is_regis = false;
        try_to_init_module = 0;
        emit signalModuleIsInitSuccess();
    }
    else{
//        debug("try_to_init_module = " + String::number(try_to_init_module));
        try_to_init_module++;
        if(try_to_init_module > 5){
            try_to_init_module = 0;
            setPwrOffModule();
        }
        else
            QTimer::singleShot(10000,this,SLOT(slotInitModule()));
    }
}

void SM_GSM_MODULE::slotSetPacketNetworkRegis()
{
    internet_is_ready = false;

    debug("## slotSetPacketNetworkRegis ");

LOOP_TRY_REGIS:

    if(!flag_packet_network_is_regis)
    {
        if(packet->getPacketNetworkRegis()){
            flag_packet_network_is_regis = true;
            try_to_packet_network_regis = 0;
            emit signalPacketNetworkIsRegis();
        }
        else{
            try_to_packet_network_regis++;
            if(try_to_packet_network_regis > 150){
                try_to_packet_network_regis = 0;
                debug("!! Warning : Packet network regis. - Timeout!!");
                emit signalTimeout();
                debug("@12");
                setPwrOffModule();
            }
            else{
                SM_DELAY::delay_ms(2000);
                goto LOOP_TRY_REGIS;
            }
        }
    }
}

void SM_GSM_MODULE::slotInitInternet(void)
{
    debug("## slotInitInternet [" + String::number(try_to_init_internet) + "]");

    internet_is_ready = false;

    try_to_init_internet++;
    if(try_to_init_internet > 5){
        QTimer::singleShot(30000,this,SLOT(slotInitModule()));
        try_to_init_internet = 0;
        return;
    }

//    debug("#A");
    if(internet->isConnect()){
        if(!internet->disConnect()){
            QTimer::singleShot(10000,this,SLOT(slotInitInternet())); //try again
            return;
        }
    }

//    debug("#B");
    if (!http->setContextid(1)){
        QTimer::singleShot(10000,this,SLOT(slotInitInternet())); //try again
        return;
    }

    if (!http->setRequestHeader(true)){
        debug("!! Warning : setRequestHeader - Timeout!!");
        return;
    }

//    debug("#C");
    if (!internet->configure()){
        QTimer::singleShot(10000,this,SLOT(slotInitInternet())); //try again
        return;
    }

    try_to_init_internet = 0;
    try_to_set_pwr_off = 0;

    slotConnectInternet();

}

void SM_GSM_MODULE::slotConnectInternet()
{
    internet_is_ready = false;

    debug("## slotConnectInternet [" + String::number(try_to_connect_internet) + "]");

    try_to_connect_internet++;

    if(try_to_connect_internet > 5){
        try_to_connect_internet = 0;
        debug("## connect_internet_false_cnt [" + String::number(connect_internet_false_cnt) + "]");
        connect_internet_false_cnt++;
        if(connect_internet_false_cnt > 2){
            connect_internet_false_cnt = 0;
            setPwrOffModule();
        }
        else
            QTimer::singleShot(30000,this,SLOT(slotInitModule())); //try again
        return;
    }

    if(!internet->isConnect()){
        debug("!! Warning : Internet don't connect");
        if(!internet->connect()){
            QTimer::singleShot(10000,this,SLOT(slotConnectInternet())); //try again
            return;
        }
    }

    connect_internet_false_cnt = 0;
    try_to_connect_internet = 0;
    internet_is_ready = true;
    emit signalInternetIsOK();
}

//void SM_GSM_MODULE::slotResetGsmModule()
//{
//    debug("Reset GsmModule");

//    for(int i=0; i < _TRY_TO_RESET_GSM_TIME; i++){
////        if(module->setPwrKey(_LOW)){
////            break;
////        }
//        if (!module->setPwrKey(_HIGH)){
//            debug("Set power - Fail");
//            //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
////            return 0;
//        }

//        if (!module->waitToReady(_WAIT_MODULE_RAEDY_TIME)) {
//            debug("Wait to gsm ready : Timeout");
//            emit signalTimeout();
//            if(!module->setURCPort(_URC_UART1)){
//                debug("Can't set URC Port !!");
//            }
//            module->saveConfig();
//            //QTimer::singleShot(2000,this,SLOT(slotInit())); //try again
////            return 0;
//        }
//        else{
//            break;
//        }
//    }

//    slotInit();
//}

//void SM_GSM_MODULE::slotResetNetwork()
//{
//    if(internet->resetConnecting())
//    {
//        emit signalResetNetworkIsOk();
//    }

//    slotInit();
//}

//void SM_GSM_MODULE::slotInit(void)
//{
//    for(int i = 0 ; i < 10; i++){
//        if(initModule()){
//            flag_gsm_module_cannot_use = false;
//            emit signalSetLEDGsm(_LED_ON);
//            return;
//        }
//    }

//    emit signalSetLEDGsm(_LED_OFF);
//    flag_gsm_module_cannot_use = true;

////    slotResetGsmModule();
//}
