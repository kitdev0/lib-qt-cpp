#include "hm_cctalk_device.h"

HM_CCTALK_DEVICE::HM_CCTALK_DEVICE(QObject *parent):
    QObject(parent)
{
    logDebug = new SM_DEBUG("ccTalkDevice");
#ifdef ACCEPTOR_PWR_PIN
    gpio.Export((char *)ACCEPTOR_PWR_PIN);
    gpio.setDirection(ACCEPTOR_PWR_PIN, (char *)"out");
#endif

#if defined(_COIN_ADDR) || defined(_BILL_ADDR)
    ccTalk = new HM_CCTALK(this);
    delay = new SM_DELAY;

    connect(ccTalk,SIGNAL(signalLogDebugSay(QString)),this,SLOT(slotLogDebugSay(QString)));
    connect(ccTalk,SIGNAL(signalTimeout()),this,SLOT(slotDeviceTimeout()));
    connect(&detectPortTimer,SIGNAL(timeout()),this,SLOT(slotDetectPort()));

#endif

#ifdef _COIN_ADDR
    connect(&coinReadBuffTimer,SIGNAL(timeout()),this,SLOT(slotReadCoinBuffer()));
    connect(ccTalk,SIGNAL(signalCoinAccepted(uint8_t)),this,SLOT(slotMicroSPCatCoinValue(uint8_t)));
    connect(ccTalk,SIGNAL(signalCoinRejected()),this,SLOT(slotCoinRejected()));
#endif

#ifdef _BILL_ADDR
    connect(&billReadBuffTimer,SIGNAL(timeout()),this,SLOT(slotReadBillBuffer()));
    connect(ccTalk,SIGNAL(signalBillVerify(uint8_t)),this,SLOT(slotITLBV20BillValue_verify(uint8_t)));
    connect(ccTalk,SIGNAL(signalBillAccepted(uint8_t)),this,SLOT(slotITLBV20BillValue_accepted(uint8_t)));
#endif
}

HM_CCTALK_DEVICE::~HM_CCTALK_DEVICE()
{
    delete logDebug;
#if defined(_COIN_ADDR) || defined(_BILL_ADDR)
    delete ccTalk;
    delete delay;
#endif
}

//public
#if defined(_COIN_ADDR) || defined(_BILL_ADDR)
void HM_CCTALK_DEVICE::connectDevice()
{
    coinAccConnectState = false;
    billAccConnectState = false;
    flagPortOpen = false;
    lastPortName = "";
    //ccTalk->ClearFlag();

    debug("Connecting...");
    portList = new QList<QSerialPortInfo>;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        portList->append(info);
    }
    if(!portList->isEmpty()){
#ifdef _COIN_ADDR
        connect(ccTalk,SIGNAL(signalReceiveFromCoin_ACK()),this,SLOT(slotSetCoinConnectState()));
#endif
#ifdef _BILL_ADDR
        connect(ccTalk,SIGNAL(signalReceiveFromBill_ACK()),this,SLOT(slotSetBillConnectState()));
#endif
        if(!detectPortTimer.isActive()) detectPortTimer.start(_DETECT_DEVICE_TIMER);
    }
}

void HM_CCTALK_DEVICE::disconnectDevice()
{
    debug("Disconnected !!");
#ifdef _COIN_ADDR
    if(coinReadBuffTimer.isActive()) coinReadBuffTimer.stop();
#endif
#ifdef _BILL_ADDR
    if(billReadBuffTimer.isActive()) billReadBuffTimer.stop();
#endif
    ccTalk->stopTimeout();
    ccTalk->closePort();
}

void HM_CCTALK_DEVICE::firmwarePath(QString _path)
{
    debug("Set firmware path >> " + _path);
    pathFile = _path;
}

//private
void HM_CCTALK_DEVICE::debug(QString data)
{
#ifdef _HM_CCTALK_DEVICE_DEBUG
#if _HM_CCTALK_DEVICE_DEBUG == _DEBUG_SAY_ONLY
    logDebug->sayln(data);
#elif _HM_CCTALK_DEVICE_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _HM_CCTALK_DEVICE_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}

void HM_CCTALK_DEVICE::slotLogDebugSay(QString _str)
{
    emit signalLogDebugSay(_str);
}

void HM_CCTALK_DEVICE::slotDetectPort()
{
    if(detectPortTimer.isActive())
        detectPortTimer.stop();

#ifdef Q_OS_OSX
    while(portList->first().portName().indexOf(String(_OSX_SERIAL_PORT_DEVICE)) == -1){
        portList->removeFirst();
        if(portList->empty())
            break;
    }
#else
#ifdef Q_OS_LINUX
    while(portList->first().portName().indexOf(String(_RPI_USB_SERIAL_PORT_DEVICE)) == -1  && portList->first().portName().indexOf(String(_RPI_ARDUINO_SERIAL_PORT_DEVICE)) == -1){
        portList->removeFirst();
        if(portList->empty())
            break;
    }
#endif
#endif

    if(!portList->empty() &&
#if defined(_COIN_ADDR) && defined(_BILL_ADDR)
    (coinAccConnectState == false || billAccConnectState == false)
#else
#ifdef _COIN_ADDR
    (coinAccConnectState == false)
#else
#ifdef _BILL_ADDR
    (billAccConnectState == false)
#endif
#endif
#endif
    )
    {
        if(lastPortName != portList->first().portName())
        {
            if(ccTalk->init(portList->first().portName())){
                flagPortOpen = true;
                _tryConnectNo = _TRY_CONNECT_NO;
                lastPortName = portList->first().portName();
            }
            else{
                portList->removeFirst();
                ccTalk->closePort();
                flagPortOpen = false;
            }
        }

        if(flagPortOpen == true && ccTalk->readyToSend())
        {
#if defined(_COIN_ADDR) && defined(_BILL_ADDR)
            if(coinAccConnectState == false)
                ccTalk->simplePoll(_COIN_ADDR);
            else if(billAccConnectState == false)
                ccTalk->simplePoll(_BILL_ADDR);
#else
#ifdef _COIN_ADDR
            if(coinAccConnectState == false)
                ccTalk->simplePoll(_COIN_ADDR);
#else
#ifdef _BILL_ADDR
            if(billAccConnectState == false)
                ccTalk->simplePoll(_BILL_ADDR);
#endif
#endif
#endif
            _tryConnectNo--;
            if(_tryConnectNo == 0){
                if(coinAccConnectState || billAccConnectState)
                    portList->clear();
                else{
                    portList->removeFirst();
                    ccTalk->closePort();
                }
            }
        }
        if(!portList->empty()){
            detectPortTimer.start(_DETECT_DEVICE_TIMER);
        }
    }
}

void HM_CCTALK_DEVICE::slotDeviceTimeout()
{
    if(coinAccConnectState || billAccConnectState)
        emit signalDeviceTimeout();
}

#ifdef _ACCEPTOR_PWR_PIN
void HM_CCTALK_DEVICE::setPower(StateType state)
{
    if(state == ON){
        if(!flag_power){
            gpio.setValue(ACCEPTOR_PWR_PIN,GPIO_HIGH);
            delay->msleep(200);
            flag_power = true;
            debug("setPower >> ON");
        }
    }
    else if(state == OFF){
        if(flag_power){
            gpio.setValue(ACCEPTOR_PWR_PIN,GPIO_LOW);
            delay->msleep(200);
            flag_power = false;
            debug("setPower >> OFF");
        }
    }

}
#endif
#endif

#ifdef _COIN_ADDR
void HM_CCTALK_DEVICE::coinAccEnable(void)
{
    debug("coinAcc >> Enable");
    if(!flag_coinEnable){
        ccTalk->setEnable(ADDR_COIN,128,127);//for MicroCoin SP --> inhibit cat1-7 and cat16
        ccTalk->setMaster_ON(ADDR_COIN);
        if(!coinReadBuffTimer.isActive()) coinReadBuffTimer.start(_COIN_INTERVAL_TM);
        flag_coinEnable = true;
    }
}

void HM_CCTALK_DEVICE::coinAccDisable(void)
{
    debug("coinAcc >> Disable");
    if(flag_coinEnable){
        ccTalk->setMaster_OFF(ADDR_COIN);
        if(coinReadBuffTimer.isActive()) coinReadBuffTimer.stop();
        flag_coinEnable = false;
    }
}

void HM_CCTALK_DEVICE::slotReadCoinBuffer(void)
{
    ccTalk->readBufEvent(ADDR_COIN);
}

//SGD Currency
void HM_CCTALK_DEVICE::slotMicroSPCatCoinValue(uint8_t cat_no)
{
    switch(cat_no){
    case 4:
        //emit signalValueCoinAccepted_satang(0.25);
        //debug("coinAcc >> Received - PBTH.");
        break;
    case 5:
        //emit signalValueCoinAccepted_satang(0.25);
        //debug("coinAcc >> Received - IBTH.");
        break;
    case 6:
        //emit signalValueCoinAccepted_satang(0.50);
        //debug("coinAcc >> Received - UBTH.");
        break;
    case 7:
        emit signalValueCoinAccepted_bath(0.1);
        debug("coinAcc >> Received - 0.1 SDG");
        break;
    case 8:
        //emit signalValueCoinAccepted_bath(1);
        //debug("coinAcc >> Received - TBTH.");
        break;
    case 9:
        emit signalValueCoinAccepted_bath(0.2);
        debug("coinAcc >> Received - 0.2 SDG.");
        break;
    case 10:
        //emit signalValueCoinAccepted_bath(2);
        //debug("coinAcc >> Received - ABTH.");
        break;
    case 11:
        emit signalValueCoinAccepted_bath(0.5);
        debug("coinAcc >> Received - 0.5 SDG");
        break;
    case 12:
        //emit signalValueCoinAccepted_bath(2);
        //debug("coinAcc >> Received - CBTH.");
        break;
    case 13:
        emit signalValueCoinAccepted_bath(1);
        debug("coinAcc >> Received - 1 SDG.");
        break;
    case 14:
        //emit signalValueCoinAccepted_bath(5);
        //debug("coinAcc >> Received - EBTH.");
        break;
    case 15:
        //emit signalValueCoinAccepted_bath(10);
        //debug("coinAcc >> Received - FBTH.");
        break;
    default :
        debug("coinAcc >> Channel not found");
    }
}
/*
void HM_CCTALK_DEVICE::slotMicroSPCatCoinValue(uint8_t cat_no)
{
    switch(cat_no){
    case 4:
        emit signalValueCoinAccepted_satang(0.25);
        debug("coinAcc >> Received - 0.25BTH.");
        break;
    case 5:
        emit signalValueCoinAccepted_satang(0.25);
        debug("coinAcc >> Received - 0.25BTH.");
        break;
    case 6:
        emit signalValueCoinAccepted_satang(0.50);
        debug("coinAcc >> Received - 0.50BTH.");
        break;
    case 7:
        emit signalValueCoinAccepted_satang(0.50);
        debug("coinAcc >> Received - 0.50BTH.");
        break;
    case 8:
        emit signalValueCoinAccepted_bath(1);
        debug("coinAcc >> Received - 1BTH.");
        break;
    case 9:
        emit signalValueCoinAccepted_bath(1);
        debug("coinAcc >> Received - 1BTH.");
        break;
    case 10:
        emit signalValueCoinAccepted_bath(2);
        debug("coinAcc >> Received - 2BTH.");
        break;
    case 11:
        emit signalValueCoinAccepted_bath(2);
        debug("coinAcc >> Received - 2BTH.");
        break;
    case 12:
        emit signalValueCoinAccepted_bath(2);
        debug("coinAcc >> Received - 2BTH.");
        break;
    case 13:
        emit signalValueCoinAccepted_bath(5);
        debug("coinAcc >> Received - 5BTH.");
        break;
    case 14:
        emit signalValueCoinAccepted_bath(5);
        debug("coinAcc >> Received - 5BTH.");
        break;
    case 15:
        emit signalValueCoinAccepted_bath(10);
        debug("coinAcc >> Received - 10BTH.");
        break;
    default :
        debug("coinAcc >> Channel not found");
    }
}
*/
void HM_CCTALK_DEVICE::slotCoinRejected()
{
    emit signalCoinRejected();
}

void HM_CCTALK_DEVICE::slotSetCoinConnectState()
{
    coinAccConnectState = true;
    disconnect(ccTalk,SIGNAL(signalReceiveFromCoin_ACK()),this,SLOT(slotSetCoinConnectState()));
    debug("Coin Acc. Connection >> OK\r\n");
    _tryConnectNo = _TRY_CONNECT_NO;
    QTimer::singleShot(_COIN_INTERVAL_TM,this,SLOT(slotResetDevice()));
    QTimer::singleShot(_COIN_INTERVAL_TM*2,this,SLOT(slotSetMaster_ON()));
    QTimer::singleShot(_COIN_INTERVAL_TM*3,this,SLOT(slotStartReadCoinBufTimer()));
    emit signalCoinAccConnected();
}

void HM_CCTALK_DEVICE::slotStartReadCoinBufTimer()
{
    coinReadBuffTimer.start(_COIN_INTERVAL_TM);
}

void HM_CCTALK_DEVICE::slotSetMaster_ON()
{
    ccTalk->setMaster_ON(_COIN_ADDR);
}

void HM_CCTALK_DEVICE::slotResetDevice()
{
    ccTalk->resetDevice(_COIN_ADDR);
}

#endif

#ifdef _BILL_ADDR
void HM_CCTALK_DEVICE::billAccEnable(void)
{
    debug("billAcc >> Enable");
    if(!flag_billEnable){
        ccTalk->setEnable(_BILL_ADDR,255,3);
        ccTalk->setMaster_ON(_BILL_ADDR);
        if(!billReadBuffTimer.isActive()) billReadBuffTimer.start(_BILL_INTERVAL_TM);
        flag_billEnable = true;
    }
}

void HM_CCTALK_DEVICE::billAccDisable(void)
{
    debug("billAcc >> Disable");
    if(flag_billEnable){
        ccTalk->setMaster_OFF(_BILL_ADDR);
        if(billReadBuffTimer.isActive()) billReadBuffTimer.stop();
        flag_billEnable = false;
    }
}

void HM_CCTALK_DEVICE::billAccAccept(void)
{
    debug("billAcc >> Accept");
    QTimer::singleShot(_BILL_INTERVAL_TM,this,SLOT(slotBillAccept()));
    QTimer::singleShot(_BILL_INTERVAL_TM*2,this,SLOT(slotStartReadBillBufTimer()));
    //billReadBuffTimer.start(BILL_INTERVAL_TM);
}

void HM_CCTALK_DEVICE::billAccReject(void)
{
    debug("billAcc >> Reject");
    ccTalk->routeBill(_BILL_ADDR,false);
}

void HM_CCTALK_DEVICE::bv20UpdateFirmware()
{
    debug("billAcc >> Update firmware");
    if(!checkFirmwareFileExist(pathFile)) {
        return;
    }

}

void HM_CCTALK_DEVICE::billReqCurrencyRev()
{
    ccTalk->reqCurrencyRev(_BILL_ADDR);
}

void HM_CCTALK_DEVICE::billReqSoftwareRev()
{
    ccTalk->reqSoftwareRev(_BILL_ADDR);
}

bool HM_CCTALK_DEVICE::checkFirmwareFileExist(QString _path_file)
{
//    QFile _file(_path_file);
    QDir _dir(_path_file);

    debug("Check firmware file exitst ?");
    debug("Path file >> " + _path_file);

    QStringList _fileslist;
    _fileslist << "*.ct1.txt";
    _fileslist << "*.ct2.txt";

    _dir.setNameFilters(_fileslist);
    QStringList _files = _dir.entryList();

    for(int i=0; i < _files.size();i++){
        debug("File name >> " + _files.at(i));
    }

    if(_files.size() == 2)
    {
        return 1;
    }
    else{
        debug("Firmware file not found!!");
        return 0;
    }
}

bool HM_CCTALK_DEVICE::readFirmwareFile(QString _path_file)
{
    QFile file(_path_file);
    if(checkFirmwareFileExist(_path_file))
    {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream( &file );
//            while( !stream.atEnd() )
//            {
//              QString text;
//              stream >> text;
//              qDebug() << text;
//            }
//            stream.seek(0);
//            QString text = stream.read(_BV20_FIEMWARE_PART_SIZE);
//            qDebug() << text;

//            stream.seek(_BV20_FIEMWARE_PART_SIZE);
//            text = stream.read(_BV20_FIEMWARE_PART_SIZE);
//            qDebug() << text;
            int _i = 0;
            while(1)
            {
                stream.seek(_i * _BV20_FIRMWARE_PART_SIZE);
                QString _data = stream.read(_BV20_FIRMWARE_PART_SIZE);
                if(_data.size() == 0){
                    debug("Read success");
                    break;
                }
                else{
                    debug("data = " + _data);
                    _i++;
                }
            }

//            while(!stream.atEnd())
//            {
//              QString text;
//              text = stream.readLine();
//              qDebug() << text;
//            }

            file.close();
            file.close();
            return 1;
        }
        else
        {
            debug("Firmware file cannot open!!");
            file.close();
            return 0;
        }
    }
    return 0;
}

//pubplic slot
void HM_CCTALK_DEVICE::slotBV20UpdateFirmware()
{
    bv20UpdateFirmware();
}

//private


//slot
void HM_CCTALK_DEVICE::slotReadBillBuffer(void)
{
    ccTalk->readBufEvent(_BILL_ADDR);
}

void HM_CCTALK_DEVICE::slotITLBV20BillValue_verify(uint8_t ch)
{
    billReadBuffTimer.stop();
    switch(ch){
    case 1:
        //emit signalValueBillVerify(20);
        billAccAccept();
        debug("billAcc >> Verified - 20BTH.");
        break;
    case 2:
        //emit signalValueBillVerify(50);
        billAccAccept();
        debug("billAcc >> Verified - 50BTH.");
        break;
    case 3:
        //emit signalValueBillVerify(100);
        billAccAccept();
        debug("billAcc >> Verified - 100BTH.");
        break;
    case 4:
        //emit signalValueBillVerify(500);
        billAccAccept();
        debug("billAcc >> Verified - 500BTH.");
        break;
    case 5:
        //emit signalValueBillVerify(500);
        billAccAccept();
        debug("billAcc >> Verified - 500BTH.");
        break;
    case 6:
        //emit signalValueBillVerify(1000);
        billAccAccept();
        debug("billAcc >> Verified - 1000BTH.");
        break;
    case 7:
        //emit signalValueBillVerify(1000);
        billAccAccept();
        debug("billAcc >> Verified - 1000BTH.");
        break;
    case 8:
        //emit signalValueBillVerify(50);
        billAccAccept();
        debug("billAcc >> Verified - 50BTH.");
        break;
    case 9:
        //emit signalValueBillVerify(20);
        billAccAccept();
        debug("billAcc >> Verified - 20BTH.");
        break;
    case 10:
        //emit signalValueBillVerify(500);
        billAccAccept();
        debug("billAcc >> Verified - 500BTH.");
        break;
    default :
        debug("billAcc >> Verified - Channel not found");
    }
}

void HM_CCTALK_DEVICE::slotITLBV20BillValue_accepted(uint8_t ch)
{
    switch(ch){
    case 1:
        emit signalValueBillAccepted(2);
        debug("billAcc >> Accepted - 20BTH.");
        break;
    case 2:
        emit signalValueBillAccepted(5);
        debug("billAcc >> Accepted - 50BTH.");
        break;
    case 3:
        emit signalValueBillAccepted(10);
        debug("billAcc >> Accepted - 100BTH.");
        break;
    case 4:
        emit signalValueBillAccepted(50);
        debug("billAcc >> Accepted - 500BTH.");
        break;
    case 5:
        emit signalValueBillAccepted(50);
        debug("billAcc >> Accepted - 500BTH.");
        break;
    case 6:
        emit signalValueBillAccepted(100);
        debug("billAcc >> Accepted - 1000BTH.");
        break;
    case 7:
        emit signalValueBillAccepted(100);
        debug("billAcc >> Accepted - 1000BTH.");
        break;
    case 8:
        emit signalValueBillAccepted(5);
        debug("billAcc >> Accepted - 50BTH.");
        break;
    case 9:
        emit signalValueBillAccepted(2);
        debug("billAcc >> Accepted - 20BTH.");
        break;
    case 10:
        emit signalValueBillAccepted(50);
        debug("billAcc >> Accepted - 500BTH.");
        break;
    default :
        debug("billAcc >> Accepted - Channel not found");
    }
}

void HM_CCTALK_DEVICE::slotSetBillConnectState()
{
    billAccConnectState = true;
    disconnect(ccTalk,SIGNAL(signalReceiveFromBill_ACK()),this,SLOT(slotSetBillConnectState()));
    debug("Bill Acc. Connection >> OK\r\n");
    _tryConnectNo = _TRY_CONNECT_NO;
//    QTimer::singleShot(_BILL_INTERVAL_TM,this,SLOT(slotSetBillMaster_ON()));
//    QTimer::singleShot(_BILL_INTERVAL_TM*2,this,SLOT(slotEnableBill()));
//    QTimer::singleShot(_BILL_INTERVAL_TM*3,this,SLOT(slotStartReadBillBufTimer()));
    emit signalBillAccConnected();
}

void HM_CCTALK_DEVICE::slotEnableBill()
{
    ccTalk->setEnable(_BILL_ADDR);
}

void HM_CCTALK_DEVICE::slotStartReadBillBufTimer()
{
    billReadBuffTimer.start(_BILL_INTERVAL_TM);
}

void HM_CCTALK_DEVICE::slotSetBillMaster_ON()
{
    ccTalk->setMaster_ON(_BILL_ADDR);
}

void HM_CCTALK_DEVICE::slotResetBill()
{
    ccTalk->resetDevice(_BILL_ADDR);
}

void HM_CCTALK_DEVICE::slotBillAccept()
{
    ccTalk->routeBill(_BILL_ADDR,true);
}


#endif
/*
delay->msleep(200);
simplePoll(COIN_ADDR);
delay->msleep(200);
selfCheck(COIN_ADDR);
delay->msleep(200);
setMaster_ON(COIN_ADDR);
delay->msleep(200);
timer.start(COIN_INTERVAL_TM);
*/

