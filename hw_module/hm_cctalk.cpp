#include "hm_cctalk.h"
//############################################################################//
//----------------Standard Message Packets, Simple checksum-------------------//
//
//  For a payload of N data bytes…
//      [ Destination Address ]
//      [ No. of Data Bytes ]
//      [ Source Address ]
//      [ Header ]
//      [ Data 1 ]
//      ...
//      [ Data N ]
//      [ Checksum ]
//
//  For a simple command or request with no data bytes…
//      [ Destination Address ]
//      [ 0 ]
//      [ Source Address ]
//      [ Header ]
//      [ Checksum ]
//
//  Simple Checksum (modulo 256)...
//          For example, the message [ 1 ] [ 0 ] [ 2 ] [ 0 ] would be followed
//      by the checksum [ 253 ] because 1 + 0 + 2 + 0 + 253 = 256 = 0.
//############################################################################//


HM_CCTALK::HM_CCTALK(QObject *parent) :
    QObject(parent)
{
    logDebug = new SM_DEBUG("ccTalk");
    timeOut_timer = new QTimer();
//    timeOut_timer->start((int)ReturnPacket_TimeOut);
    //connect(ui->selectBt_item1,SIGNAL(clicked()),this,SLOT(slotRead_item1Detials()));
    //if(!polling_timer.isActive()) polling_timer.start(Polling_sendPacketTime);
    //connect(&polling_timer,SIGNAL(timeout()),this,SLOT(slotSendPacket()));
    connect(timeOut_timer,SIGNAL(timeout()),this,SLOT(slotTimeOut()));
    connect(this,SIGNAL(signalReceivedPacket(QByteArray)),this,SLOT(slotChReceivePacket(QByteArray)));
    connect(logDebug,SIGNAL(signalSay(QString)),this,SLOT(slotLogDebugSay(QString)));
}

HM_CCTALK::~HM_CCTALK()
{
#ifdef _HM_CCTALK_DEBUG
    delete logDebug;
#endif
}

void HM_CCTALK::debug(QString data)
{
#ifdef _HM_CCTALK_DEBUG
#if _HM_CCTALK_DEBUG == _DEBUG_SAY_ONLY
    logDebug->sayln(data);
#elif _HM_CCTALK_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _HM_CCTALK_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}
//------ public  ------//
bool HM_CCTALK::init()
{
    debug("Initial port");
    if(moduleInit(_CCTALK_PORT))
        return true;
    else
        return false;
}

bool HM_CCTALK::init(QString serial_port)
{
    debug("Initial port");
    if(moduleInit(serial_port))
        return true;
    else
        return false;
}

void HM_CCTALK::simplePoll(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 0;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = SIMPLE_POLL;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder);
    debug("simplePoll addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::selfCheck(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 0;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = PERFORM_SELF_CHECK;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder);
    debug("selfCheck addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::setEnable(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 2;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = MOD_INHIBIT_STATUS;
    if(slave == ADDR_BILL){
        SPacket.Data[0] = 255;
        SPacket.Data[1] = 3;
    }
    else if(slave == ADDR_COIN){
        SPacket.Data[0] = 248;
        SPacket.Data[1] = 127;
    }

    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder+SPacket.Data[0]+SPacket.Data[1]);
    debug("setEnable addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::setEnable(ccTalkAddressType slave, uint8_t data0, uint8_t data1)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 2;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = MOD_INHIBIT_STATUS;
    if(slave == ADDR_BILL){
        SPacket.Data[0] = data0;
        SPacket.Data[1] = data1;
    }
    else if(slave == ADDR_COIN){
        SPacket.Data[0] = data0;
        SPacket.Data[1] = data1;
    }

    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder+SPacket.Data[0]+SPacket.Data[1]);
    debug("setEnable addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::setMaster_ON(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 1;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = MOD_MASTER_INTHIBIT_STATUS;
    SPacket.Data[0] = 1;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder+SPacket.Data[0]);
    debug("setMaster_ON addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::setMaster_OFF(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 1;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = MOD_MASTER_INTHIBIT_STATUS;
    SPacket.Data[0] = 0;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder+SPacket.Data[0]);
    debug("setMaster_OFF addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::readBufEvent(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 0;
    SPacket.Soure_Adds = ADDR_MASTER;
    if(slave == ADDR_BILL)
        SPacket.Hearder = READ_BUFFER_BILL_EVENT;
    else if(slave == ADDR_COIN)
        SPacket.Hearder = READ_BUFFER_COIN;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder);
    sendSimplePacket();
}

void HM_CCTALK::routeBill(ccTalkAddressType slave,bool dir)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 1;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = ROUTE_BILL;
    if(dir == true)
        SPacket.Data[0] = 1;
    else
        SPacket.Data[0] = 0;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder+SPacket.Data[0]);
    debug("routeBill");
    sendSimplePacket();
}

void HM_CCTALK::reqSoftwareRev(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 0;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = REQ_SOFTWARE_REVISION;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder);
    debug("reqSoftwareRev. addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::reqCurrencyRev(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 0;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = REQ_CURRENCY_REVISION;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder);
    debug("reqCurrentRev. addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

void HM_CCTALK::resetDevice(ccTalkAddressType slave)
{
    SPacket.Dest_Adds = slave;
    SPacket.No_Data = 0;
    SPacket.Soure_Adds = ADDR_MASTER;
    SPacket.Hearder = RESET_DEVICE;
    SPacket.ChSum = calSimpleChSum(SPacket.Dest_Adds+SPacket.No_Data+SPacket.Soure_Adds+SPacket.Hearder);
    debug("resetDevice addr. >> " + QString("%1").arg((int)slave ,0 ,10));
    sendSimplePacket();
}

bool HM_CCTALK::connectState()
{
    return flagConnectState;
}

bool HM_CCTALK::readyToSend()
{
    return flagReadyToSend;
}

void HM_CCTALK::closePort()
{
    if(ccTalkPort->isOpen()){
        ccTalkPort->close();
        SM_DELAY::delay_ms(100);
        clearFlag();
    }
}

void HM_CCTALK::clearFlag()
{
    read_packet_buf.clear();
    flagConnectState = false;
    flagReadyToSend = true;
}

void HM_CCTALK::stopTimeout()
{
    if(timeOut_timer->isActive()) timeOut_timer->stop();
}

//public//

//private
bool HM_CCTALK::moduleInit(QString port_name)
{
    ccTalkPort = new QSerialPort();
    ccTalkPort->setPortName(port_name);
    ccTalkPort->setBaudRate(QSerialPort::Baud9600);
    ccTalkPort->setParity(QSerialPort::NoParity);
    debug("Opening... >> " + port_name);

    if(ccTalkPort->isOpen() == true){
        ccTalkPort->close();
        SM_DELAY::delay_ms(100);
    }

    if (ccTalkPort->open(QIODevice::ReadWrite)){
        debug("Open port >> passed");
        flagReadyToSend = true;
        flagConnectState = false;
        connect(ccTalkPort, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        return true;
    }
    else{
        debug("Open port >> failed");
        disconnect(ccTalkPort, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        ccTalkPort->close();
        SM_DELAY::delay_ms(100);
        return false;
    }
}

void HM_CCTALK::writeData(QByteArray data, u_int8_t size)
{
    QString str;
    for (int i = 0; i < size; i++) {
        str += QString("%1 ").arg((u_int8_t)data[i] ,0 ,10);
    }
    debug("send packet >> " + str);

    reflect_data_size = size;
    if((u_int8_t)data[0] == ADDR_COIN){
        lastCoinHead_send = (u_int8_t)data[3];
    }
    else if((u_int8_t)data[0] == ADDR_BILL)
        lastBillHead_send = (u_int8_t)data[3];
    if(ccTalkPort->isWritable()){
        ccTalkPort->write(data, size);
        while(!ccTalkPort->atEnd());
        if(!timeOut_timer->isActive()) timeOut_timer->start(_CCTALK_RESTURN_PACKET_TIMEOUT);
    }
    else
        debug("cannot write data");
}

char HM_CCTALK::calSimpleChSum(char raw)
{
    raw = 256*((raw/256)+1)-raw;
    return raw;
}

void HM_CCTALK::sendSimplePacket()
{
    if (flagReadyToSend == true){
        flagReadyToSend = false;
        QByteArray packet_buf;
        packet_buf.clear();

        packet_buf += SPacket.Dest_Adds;
        packet_buf += SPacket.No_Data;
        packet_buf += SPacket.Soure_Adds;
        packet_buf += SPacket.Hearder;
        for(uint8_t j=0; j<SPacket.No_Data; j++){
            packet_buf += SPacket.Data[j];
        }
        packet_buf += SPacket.ChSum;
        //send_packet_buf.append(packet_buf,i);
        writeData(packet_buf,packet_buf.size());
    }
    //else
        //debug("!!Error data sending!!");
}

void HM_CCTALK::recordData(QByteArray data)
{
    for(int i=0; i < data.size();i++){
        if(reflect_data_size)
            reflect_data_size--; //reflect filter
        else
            read_packet_buf += data[i];
    }
    if(read_packet_buf.size() == read_packet_buf[1]+5){
        QString str;
        for (int i = 0; i < read_packet_buf.size(); i++) {
            str += QString("%1 ").arg((u_int8_t)read_packet_buf[i] ,0 ,10);
        }
        //debug("receive packet >> " + str);
        checkPacketFormat(read_packet_buf);
        read_packet_buf.clear();
        if(timeOut_timer->isActive()) timeOut_timer->stop();
    }
    /*
    for (int i = 0; i < size;i++) {
        if((int)ucae_comm_buf[0] == P_SYNC){
            ucae_comm_buf += data[i];
            if(ucae_comm_buf.size() == (int)ucae_comm_buf[1] && ucae_comm_buf.size() >= P_LNG5){
                if(ChSum(ucae_comm_buf)){
                    checkComm(ucae_comm_buf);
                    //qDebug() << "ucae check command";
                }
                ucae_comm_buf.clear();
            }
        }
        else if(data[i] == (char)P_SYNC && ucae_comm_buf.isEmpty()){
            //qDebug() << "P_StartByteComm";
            ucae_comm_buf += data[i];
        }
    }
    */

}

bool HM_CCTALK::simpleChSum(QByteArray row)
{
    char sumPacket=0;
    for(int i=0; i < (row.size()-1); i++){
        sumPacket += row[i];
    }

    if((char)row[row.size()-1] == calSimpleChSum(sumPacket)){
        flagConnectState = true;
        //debug("simpleChSum >> OK");
        return true;
    }
    else{
        debug("simpleChSum >> failed");
        flagConnectState = false;
        return false;
    }
}

void HM_CCTALK::checkPacketFormat(QByteArray packet)
{
    if((int)packet[0] == ADDR_MASTER){
        if(simpleChSum(packet))
            emit signalReceivedPacket(packet);
        else
            debug("error Packet ChSum is wrong !!");
    }
    else
        debug("error ADDR_MASTER is wrong !!");
}

void HM_CCTALK::checkCoinPacketData(QByteArray packet_data)
{
    if(lastCoinHead_send == (u_int8_t)READ_BUFFER_COIN){
        if((uint8_t)packet_data[5] != 0){
            if(event_Coin != (uint8_t)packet_data[4])
            {
                emit signalCoinAccepted((uint8_t)packet_data[5]);
                event_Coin = (uint8_t)packet_data[4];
            }
        }
        else{
            if(event_Coin != (uint8_t)packet_data[4])
            {
                emit signalCoinRejected();
                event_Coin = (uint8_t)packet_data[4];
            }
        }
    }
    else if(lastCoinHead_send == (u_int8_t)PERFORM_SELF_CHECK){
        if((uint8_t)packet_data[4] == 0)
            debug("Coin-Self-Check -> OK");
        else{
            debug("Coin-Self-Check -> ERROR-Code = " + QString("%1").arg((u_int8_t)packet_data[4] ,0 ,10));
            emit signalCoinSelfCheckError((uint8_t)packet_data[4]);
        }
    }
    else{
        debug("!!warning - Received CoinData don't diagnose");
        debug("#dataC = " + QString("%1 ").arg((u_int8_t)lastCoinHead_send ,0 ,10));
        debug("#dataD = " + QString("%1 ").arg((u_int8_t)READ_BUFFER_COIN ,0 ,10));

    }
}

//void HM_CCTALK::checkBillPacketData(QByteArray packet_data)
//{
//    if(lastBillHead_send == READ_BUFFER_BILL_EVENT)
//    {
//        /*
//        if((uint8_t)packet_data[6] == 1)
//        {
//            if(event_Bill != (uint8_t)packet_data[4]){
//                emit signalBillVerify((uint8_t)packet_data[5]);
//                qDebug() << "emit signalBillVerify";
//            }
//            if((uint8_t)packet_data[5] != 0){
//                if(event_Bill != (uint8_t)packet_data[4]){
//                    emit signalBillAccepted((uint8_t)packet_data[5]);
//                    event_Bill = (uint8_t)packet_data[4];
//                    qDebug() << "signalBillAccepted";
//                }
//            }
//        }
//        */
//        if(event_Bill != (uint8_t)packet_data[4])
//        {
//            if((uint8_t)packet_data[5] != 0)
//            {
//                if((uint8_t)packet_data[6] == 1){
//                    emit signalBillVerify((uint8_t)packet_data[5]);
//                    debug("emit signalBillVerify");
//                }
//                else if((uint8_t)packet_data[6] == 0){
//                    emit signalBillAccepted((uint8_t)packet_data[5]);
//                    debug("emit signalBillAccepted");
//                }
//            }
//            event_Bill = (uint8_t)packet_data[4];
//        }

//    }
//    else if(lastBillHead_send == PERFORM_SELF_CHECK){
//        if((uint8_t)packet_data[4] == 0)
//            debug("Bill-Self-Check -> OK");
//        else{
//            debug("Bill-Self-Check -> ERROR-Code = " + QString("%1").arg((int)packet_data[4] ,0 ,10));
//            emit signalBillSelfCheckError((uint8_t)packet_data[4]);
//        }
//    }
//    else
//        debug("!!warning - Received BillData don't diagnose");
//}

void HM_CCTALK::checkBillPacketData(QByteArray packet_data)
{
    QString _str = "";

    switch (lastBillHead_send)
    {
    case REQ_SOFTWARE_REVISION :
        break;
    case REQ_CURRENCY_REVISION :
        _str = _str.mid(4,(int)packet_data[1]);
        debug("Currency Rev. = " + _str);
        break;
    case READ_BUFFER_BILL_EVENT :
        if(event_Bill != (uint8_t)packet_data[4])
        {
            if((uint8_t)packet_data[5] != 0)
            {
                if((uint8_t)packet_data[6] == 1){
                    emit signalBillVerify((uint8_t)packet_data[5]);
                    debug("emit signalBillVerify");
                }
                else if((uint8_t)packet_data[6] == 0){
                    emit signalBillAccepted((uint8_t)packet_data[5]);
                    debug("emit signalBillAccepted");
                }
            }
            event_Bill = (uint8_t)packet_data[4];
        }
        break;

    case PERFORM_SELF_CHECK:
        if((uint8_t)packet_data[4] == 0)
            debug("Bill-Self-Check -> OK");
        else{
            debug("Bill-Self-Check -> ERROR-Code = " + QString("%1").arg((int)packet_data[4] ,0 ,10));
            emit signalBillSelfCheckError((uint8_t)packet_data[4]);
        }
        break;
    default:
        debug("!!warning - Received BillData don't diagnose");
        break;
    }
}
//private//

//------ private slot ------//
void HM_CCTALK::slotReadyRead()
{
    QString str;
    QByteArray read_buf;
    uint dataAvailable;
    while(ccTalkPort->bytesAvailable()){
        dataAvailable = ccTalkPort->bytesAvailable();
        read_buf += ccTalkPort->read(dataAvailable);
    }

//    for (int i = 0; i < read_buf.size(); i++) {
//        str += QString("%1 ").arg((u_int8_t)read_buf[i] ,0 ,10);
//    }

//    logDebug->Print("read data >> " + str);
    recordData(read_buf);

}

void HM_CCTALK::slotChReceivePacket(QByteArray return_packet)
{
    if((int)return_packet[2] == ADDR_COIN)
    {
        if((int)return_packet[1]){ //data > 0
            checkCoinPacketData(return_packet);
        }
        else if((int)return_packet[3] == 0){
            //debug("ReceiveFromCoin - ACKED...");
            emit signalReceiveFromCoin_ACK();
        }
        else if((int)return_packet[3] == 5){
            debug("ReceiveFromCoin - NACKED...");
            emit signalReceiveFromCoin_NACK();
        }
        else
            debug("!! warning - Received CoinHeader undefine");

    }
    else if((int)return_packet[2] == ADDR_BILL){
        if((int)return_packet[1]){ //data > 0
            checkBillPacketData(return_packet);
        }
        else if((int)return_packet[3] == 0){
            //debug("ReceiveFromBill - ACKED...");
            emit signalReceiveFromBill_ACK();
        }
        else if((int)return_packet[3] == 5){
            //debug("ReceiveFromBill - NACKED...");
            emit signalReceiveFromBill_NACK();
        }
        else
            debug("!! warning - Received BillHeader undefine");
    }
    flagReadyToSend = true;
}

void HM_CCTALK::slotTimeOut()
{
    debug("--> Time out. Please check device !!");
    if(timeOut_timer->isActive()) timeOut_timer->stop();
    emit signalTimeout();
    flagConnectState = false;
    flagReadyToSend = true;
}

//void HM_CCTALK::slotSendPacket()
//{
//    if(flagReadyToSend && send_packet_buf.size()){
//        char packet_buf[MAX_NO_DATA_BYTE];
//        int endlen = send_packet_buf[1]+5;
//        flagReadyToSend = false;

//        QString str;
//        for (int i = 0; i < endlen; i++) {
//            str += QString("%1 ").arg((int)send_packet_buf[i] ,0 ,10);
//        }
//        debug("send_packet = " + str);

//        for(int i=0;i < endlen;i++){
//            packet_buf[i] = send_packet_buf[i];
//        }

//        writeData(packet_buf,endlen);
//        send_packet_buf.remove(0,(endlen));
//        //qDebug() << "#slotSendPacket";
//    }
//}

void HM_CCTALK::slotLogDebugSay(QString _str)
{
    emit signalLogDebugSay(_str);
}

//private slots//
