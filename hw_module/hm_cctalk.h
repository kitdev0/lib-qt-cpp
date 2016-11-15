#ifndef HM_CCTALK_H
#define HM_CCTALK_H

#include <QObject>
#include <QStringList>
#include <QThread>
#include <inttypes.h>
#include <QSerialPort>
#include <QTimer>
#include <sw_module/sm_debug.h>
#include <sw_module/sm_delay.h>

#ifndef _DEBUG_SAY_ONLY
#define _DEBUG_SAY_ONLY 0
#endif //_DEBUG_SAY_ONLY

#ifndef _DEBUG_WRITE_ONLY //Set write to se card
#define _DEBUG_WRITE_ONLY 1
#endif //_DEBUG_WRITE_ONLY

#ifndef _DEBUG_SAY_AND_WRITE
#define _DEBUG_SAY_AND_WRITE 2
#endif //_DEBUG_PRINT_AND_WRITE

#ifdef Q_OS_OSX
#define _HM_CCTALK_DEBUG _DEBUG_SAY_ONLY
#else
//#define _HM_CCTALK_DEBUG _DEBUG_SAY_ONLY
//#define _HM_CCTALK_DEBUG _DEBUG_WRITE_ONLY
#define _HM_CCTALK_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#define _CCTALK_MAX_NO_DATA_BYTE 4

#ifndef _CCTALK_PORT
#define _CCTALK_PORT "/dev/ttyS2"
#endif

#define _CCTALK_RESTURN_PACKET_TIMEOUT 500

enum ccTalkHeaderType
{
    //FACTORY_SETUP_AND_TEST  = 255,
    SIMPLE_POLL             = 254,
    ADDR_POLL               = 253,
    //ADDR_CLASH              = 252,
    //ADDR_CHANGE             = 251,
    //ADDR_RANDOM             = 250,
    //REQ_POLLING_PRIORITY    = 249,
    //REQ_STATUS              = 248,
    //REQ_VARIABLE_SET        = 247,
    //REQ_MANUFACTURER_ID     = 246,
    //REQ_EQUIPMENT_CATE_ID   = 245,
    //REQ_PRODUCT_CODE        = 244,
    //REQ_DATABASE_VERSION    = 243,
    //REQ_SERIAL_NUMBER       = 242,
    REQ_SOFTWARE_REVISION   = 241,
    //TEST_SOLENOIDS          = 240,
    //OPERATE_MOTORS          = 239,
    //TEST_OUTPUT_LINES       = 238,
    //READ_INPUT_LINE         = 237,
    //READ_OPTO_STATES        = 236,
    //READ_DH_PUBLIC_KEY      = 235,
    //SEND_DH_PUBLIC_KEY      = 234,
    //LATCH_OUTPUT_LINE       = 233,
    PERFORM_SELF_CHECK      = 232,
    MOD_INHIBIT_STATUS      = 231,
    //REQ_INHIBIT_STATUS      = 230,
    READ_BUFFER_COIN        = 229,
    MOD_MASTER_INTHIBIT_STATUS  = 228,
    //REQ_MASTER_INTHIBIT_STATUS  = 227,
    //REQ_INSERT_COUNTER      = 226,
    //REQ_ACCEPT_COUNTER      = 225,
    //REQ_ENCRYP_PRODUCT_ID   = 224,
    //MOD_ENCRYP_INHIBIT      = 223,
    //MOD_SORTER_OVERRIDE_STATUS  = 222,
    //REQ_SORTER_OVERRIDE_STATUS  = 221,
    //ACMI_ENCRYP_DATA        = 220,
    //ENTER_NEW_PIN_NUMBER    = 219,
    //ENTER_PIN_NUMBER        = 218,
    //REQ_PAYOUT_HIGH_LOW_STATUS  = 217,
    //REQ_DATA_STORAGE_AVAILABILITY   = 216,
    //READ_DATA_BLOCK         = 215,
    //WRITE_DATA_BLOCK        = 214,
    //REQ_OPTION_FLAGS        = 213,
    //REQ_COIN_POSITION       = 212,
    //POWER_MANAGE_CONTROL    = 211,
    //MOD_SORTER_PATHS        = 210,
    //REQ_SORTER_PATHS        = 209,
    //MOD_PAYOUT_APSOL_COUNT  = 208,
    //REQ_PAYOUT_APSOL_COUNT  = 207,
    //METER_CONTROL           = 204,
    //DISPLAY_CONTROL         = 203,
    //TEACH_MODE_CONTROL      = 202,
    //REQ_TEACH_STATUS        = 201,
    //ACMI_UNENCRYP_PRODUCT_ID    = 200,
    //CONFIG_TO_EEPROM        = 199,
    //COUNTER_TO_EEPROM       = 198,
    //CAL_ROM_CHECKSUM        = 197,
    //REQ_CREATION_DATE       = 196,
    //REQ_LAST_MOD_DATE       = 195,
    //REQ_REJECT_COUNTER      = 194,
    //REQ_FRAUD_COUNTER       = 193,
    //REQ_BUILD_CODE          = 192,
    //KEYPAD_CONTROL          = 191,
    //MOD_DEFAULT_SORTER_PATH = 189,
    //REQ_DEFAULT_SORTER_PATH = 188,
    //MOD_PAYOUT_CAPACITY     = 187,
    //REQ_PAYOUT_CAPACITY     = 186,
    //MOD_COIN_ID             = 185,
    //REQ_COIN_ID             = 184,
    //UPLOAD_WINDOW_DATA      = 183,
    //DOWNLOAD_CALIBRAT_INFO  = 182,
    //MOD_SECURITY_SETTING    = 181,
    //REQ_SECURITY_SETTING    = 180,
    //MOD_BANK_SELECT         = 179,
    //REQ_BANK_SELECT         = 178,
    //HANDHELD_FUNCTION       = 177,
    //REQ_ALARM_COUNTER       = 176,
    //MOD_PAYOUT_FLOAT        = 175,
    //REQ_PAYOUT_FLOAT        = 174,
    //REQ_THERMISTOR_READING  = 173,
    //EMER_STOP               = 172,
    //REQ_HOPPER_COIN         = 171,
    //REQ_BASE_YEAR           = 170,
    //REQ_ADDRESS_MODE        = 169,
    //REQ_HOPPER_DISPENSE_COUNT   = 168,
    //DISPENSE_HOPPER_COIN    = 167,
    //REQ_HOPPER_STATUS       = 166,
    //MOD_VARIABLE_SET        = 165,
    //ENABLE_HOPPER           = 164,
    //TEST_HOPPER             = 163,
    //MOD_INHIBIT_AND_OVERRIDE_REG    = 162,
    //PUMP_RNG                = 161,
    //REQ_CIPHER_KEY          = 160,
    READ_BUFFER_BILL_EVENT  = 159,
    //MOD_BILL_ID             = 158,
    //REQ_BILL_ID             = 157,
    //REQ_COUNTRY_SCALING_FACTOR  = 156,
    //REQ_BILL_POSITION       = 155,
    ROUTE_BILL              = 154,
    //MOD_BILL_OPERATING_MODE = 153,
    //RED_BILL_OPERATING_MODE = 152,
    //TEST_LAMPS              = 151,
    //REQ_INDIV_ACCEPT_COUNTER    = 150,
    //REQ_INDIV_ERROR_COUNTER = 149,
    //READ_OPTO_VOLTAGES      = 148,
    //PERFORM_STACKER_CYCLE   = 147,
    //OPERATE_BI_DIRECT_MOTORS    = 146,
    REQ_CURRENCY_REVISION   = 145,
    //UPLOAD_BILL_TABLE       = 144,
    //BEGIN_BILL_TABLE_UPGRADE    = 143,
    //FINISH_BILL_TABLE_UPGRADE   = 142,
    //REQ_FW_UPGRAGE_CAPABILITY   = 141,
    UPLOAD_FW               = 140,
    BEGIN_FW_UPGRADE        = 139,
    FINISH_FW_UPGRADE       = 138,
    //SWITCH_ENCRYP_CODE      = 137,
    //STORE_ENCRYP_CODE       = 136,
    //SET_ACCEPT_LIMIT        = 135,
    //DISPENSE_HOPPER_VALUE   = 134,
    //REQ_HOPPER_POLL_VALUE   = 133,
    //EMER_STOP_VALUE         = 132,
    //REQ_HOPPER_COIN_VALUE   = 131,
    //REQ_INDEX_HOPPER_DISPENSE_COUNT = 130,
    //READ_BARCODE_DATA       = 129,
    //REQ_MONEY_IN            = 128,
    //REQ_MONEY_OUT           = 127,
    //CLEAR_MONEY_COUNTERS    = 126,
    //PAY_MONEY_OUT           = 125,
    //VERIFY_MONEY_OUT        = 124,
    //REQ_ACTIVITY_REGISTER   = 123,
    //REQ_ERROR_STATUS        = 122,
    //PURGE_HOPPER            = 121,
    //MOD_HOPPER_BALANCE      = 120,
    //REQ_HOPPER_BALANCE      = 119,
    //MOD_CASHBOX_VALUE       = 118,
    //REQ_CASHBOX_VALUE       = 117,
    //MOD_REALTIME_CLOCK      = 116,
    //REQ_REALTIME_CLOCK      = 115,
    //REQ_USB_ID              = 114,
    SWITCH_BAUD_RATE        = 113,
    //READ_ENCRYP_EVENTS      = 112,
    //REQ_ENCRYP_SUPPORT      = 111,
    //SWITCH_ENCRYP_KEY       = 110,
    //REQ_ENCRYP_HOPPER_STATUS    = 109,
    //REQ_ENCRYP_MONETARY_ID  = 108,
    //OPERATE_ESCROW          = 107,
    //REQ_ESCROW_STATUS       = 106,
    //DATA_STREAM             = 105,
    //REQ_SERVICE_STATUS      = 104,
    //EXPANSION_HEADER_4      = 103,
    //EXPANSION_HEADER_3      = 102,
    //EXPANSION_HEADER_2      = 101,
    //EXPANSION_HEADER_1      = 100,
    FW_UPGRADE_ERROR_DESCRIPYION    = 99,
    //BUSY_MESSAGE            = 6,
    NAK_MESSAGE             = 5,
    //REQ_COMMS_REVISION      = 4,
    //CLEAR_COMMS_STATUS_VARIABLES    = 3,
    //REQ_COMMS_STATUS_VARIABLES  = 2,
    RESET_DEVICE            = 1,
    RETURN_MESSAGE          = 0
};

enum ccTalkAddressType
{
    ADDR_MASTER = 1,
    ADDR_BILL   = 40,
    ADDR_COIN   = 2
};

struct ccTalkComm_t
{
    ccTalkAddressType Dest_Adds;
    char        No_Data;
    ccTalkAddressType Soure_Adds;
    ccTalkHeaderType  Hearder;
    char        Data[_CCTALK_MAX_NO_DATA_BYTE];
    char        ChSum;
};

class HM_CCTALK : public QObject
{
    Q_OBJECT
public:
    explicit HM_CCTALK(QObject *parent = 0);
    ~HM_CCTALK();

    bool init();
    bool init(QString serial_port);
    bool init(const QSerialPortInfo&);
    void simplePoll(ccTalkAddressType slave);
    void selfCheck(ccTalkAddressType slave);
    void setEnable(ccTalkAddressType slave);
    void setEnable(ccTalkAddressType slave, uint8_t data0, uint8_t data1);
    void setMaster_ON(ccTalkAddressType slave);
    void setMaster_OFF(ccTalkAddressType slave);
    void readBufEvent(ccTalkAddressType slave);
    void resetDevice(ccTalkAddressType slave);
    void routeBill(ccTalkAddressType slave,bool dir);
    void reqSoftwareRev(ccTalkAddressType slave);
    void reqCurrencyRev(ccTalkAddressType slave);
    bool connectState();
    bool readyToSend();
    void closePort();
    void clearFlag();
    void stopTimeout();
signals:
    void signalReceivedPacket(QByteArray packet);
    void signalReceiveFromCoin_ACK(void);
    void signalReceiveFromCoin_NACK(void);
    void signalReceiveFromBill_ACK(void);
    void signalReceiveFromBill_NACK(void);
    void signalTimeout(void);
    void signalCoinAccepted(uint8_t);
    void signalCoinRejected();
    void signalCoinSelfCheckError(uint8_t);
    void signalBillVerify(uint8_t);
    void signalBillAccepted(uint8_t);
    void signalBillSelfCheckError(uint8_t);
    void signalLogDebugSay(QString);
public slots:

private:
    QSerialPort *ccTalkPort;
    ccTalkComm_t SPacket;//Simple Packet
    QTimer *timeOut_timer;
    //QTimer polling_timer;
    SM_DEBUG *logDebug;

    QByteArray read_packet_buf;
    QByteArray send_packet_buf;

    int16_t reflect_data_size = 0;
    int16_t lastCoinHead_send = RETURN_MESSAGE;
    int16_t lastBillHead_send = RETURN_MESSAGE;

    bool flagConnectState = false;
    bool flagReadyToSend = true;
    bool flagCcTalkPortSetMem = true;

    uint8_t event_Coin = 0;
    uint8_t event_Bill = 0;

    bool moduleInit(QString);
    void writeData(QByteArray data, uint8_t size);
    char calSimpleChSum(char raw);
    void sendSimplePacket(void);
    void recordData(QByteArray data);
    bool simpleChSum(QByteArray row);
    void checkPacketFormat(QByteArray read_packet_buf);
    void checkCoinPacketData(QByteArray);
    void checkBillPacketData(QByteArray);
    void tryToConnect(QString _comPort);
    void debug(QString data);
private slots:
    void slotReadyRead(void);
    void slotChReceivePacket(QByteArray);
    void slotTimeOut(void);
//    void slotSendPacket(void);
    void slotLogDebugSay(QString _str);

};

#endif // HM_CCTALK_H
