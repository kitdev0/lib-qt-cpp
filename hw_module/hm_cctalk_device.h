#ifndef HM_CCTALK_DEVICE_H
#define HM_CCTALK_DEVICE_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QSerialPortInfo>
#include <QProcess>
#include "sw_module/sm_delay.h"
#include "hw_module/hm_cctalk.h"

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
#define _HM_CCTALK_DEVICE_DEBUG _DEBUG_SAY_ONLY
#else
//#define _HM_CCTALK_DEVICE_DEBUG _DEBUG_SAY_ONLY
//#define _HM_CCTALK_DEVICE_DEBUG _DEBUG_WRITE_ONLY
#define _HM_CCTALK_DEVICE_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#define _COIN_INTERVAL_TM    200
#define _BILL_INTERVAL_TM    100
#define _DETECT_DEVICE_TIMER 80
#define _TRY_CONNECT_NO      3

//#define _COIN_ADDR ADDR_COIN
#define _BILL_ADDR ADDR_BILL

//#define _ACCEPTOR_PWR_PIN  "192"
#ifdef _ACCEPTOR_PWR_PIN
#include "hw_module/hm_gpio_v1_01.h"
#endif

#ifndef _OSX_SERIAL_PORT_DEVICE
#define _OSX_SERIAL_PORT_DEVICE         "tty.usbserial"
#endif

#ifndef _RPI_USB_SERIAL_PORT_DEVICE
#define _RPI_USB_SERIAL_PORT_DEVICE     "ttyUSB"
#endif

#ifndef _RPI_ARDUINO_SERIAL_PORT_DEVICE
#define _RPI_ARDUINO_SERIAL_PORT_DEVICE "ttyACM"
#endif

#ifndef _FIRMWARE_PATH
#define _FIRMWARE_PATH  "/Users/kitdev/CloudBoxApp/BV20firmware/" //Setting Firmware path
#endif

#ifndef _BV20_FIRMWARE_NAME_1
#define _BV20_FIRMWARE_NAME_1   "THB01215_BV00204182328000_IF_01.ct1.txt"
#endif

#ifndef _BV20_FIRMWARE_NAME_2
#define _BV20_FIRMWARE_NAME_2   "THB01215_BV00204182328000_IF_01.ct2.txt"
#endif

#ifndef _BV20_FIRMWARE_PART_SIZE
#define _BV20_FIRMWARE_PART_SIZE    128
#endif

enum StateType
{
    ON,
    OFF
};

class HM_CCTALK_DEVICE : public QObject
{
    Q_OBJECT
public:
    explicit HM_CCTALK_DEVICE(QObject *parent = 0);
    ~HM_CCTALK_DEVICE();

#if defined(_COIN_ADDR) || defined(_BILL_ADDR)
public:
    void connectDevice(void);
    void disconnectDevice(void);
    void firmwarePath(QString _path);
private:
    HM_CCTALK *ccTalk;
    SM_DEBUG *logDebug;
    QList<QSerialPortInfo> *portList;
    QTimer detectPortTimer;
    SM_DELAY *delay;
#ifdef _ACCEPTOR_PWR_PIN
    HM_GPIO_V1_01 gpio;
public:
    void setPower(StateType state);
#endif
    QString lastPortName = "";
    QString pathFile = _FIRMWARE_PATH;

    uint8_t _tryConnectNo = _TRY_CONNECT_NO;

    bool flag_power = false;
    bool flag_coinEnable = false;
    bool flag_billEnable = false;
    bool coinAccConnectState = false;
    bool billAccConnectState = false;
    bool flagPortOpen = false;

    void debug(QString data);

private slots:
    void slotLogDebugSay(QString);
    void slotDetectPort();
    void slotDeviceTimeout();
signals:
    void signalLogDebugSay(QString);
    void signalDeviceTimeout();
#endif

#ifdef _COIN_ADDR
public:
    void coinAccEnable(void);
    void coinAccDisable(void);
private:
    QTimer coinReadBuffTimer;
private slots:
    void slotReadCoinBuffer(void);
    void slotMicroSPCatCoinValue(uint8_t);
    void slotCoinRejected();
    void slotSetCoinConnectState();
    void slotStartReadCoinBufTimer();
    void slotSetMaster_ON();
    void slotResetDevice();
signals:
    void signalValueCoinAccepted_satang(float);
    void signalValueCoinAccepted_bath(float);
    void signalCoinRejected();
    void signalCoinAccConnected();
#endif

#ifdef _BILL_ADDR
public:
    void billAccEnable(void);
    void billAccDisable(void);
    void billAccAccept(void);
    void billAccReject(void);
    void bv20UpdateFirmware(void);
    QTimer billReadBuffTimer;
private:

private slots:
    void slotReadBillBuffer(void);
    void slotITLBV20BillValue_verify(uint8_t);
    void slotITLBV20BillValue_accepted(uint8_t);
    void slotSetBillConnectState(void);
    void slotEnableBill();
    void slotStartReadBillBufTimer();
    void slotSetBillMaster_ON();
    void slotResetBill();
    void slotBillAccept();
signals:
    void signalValueBillVerify(int);
    void signalValueBillAccepted(int);
    void signalBillAccConnected();
#endif
};

#endif // HM_CCTALK_DEVICE_H
