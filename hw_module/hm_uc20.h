#ifndef _HM_UC20_H
#define _HM_UC20_H

#include "../../../../lib-qt-cpp/sw_module/sm_debug.h"
#include "../../../../lib-qt-cpp/sw_module/sm_delay.h"
#include "hm_gpio.h"

#include <QElapsedTimer>
#include <QSerialPortInfo>
#include <QList>

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
#define _UC20_DEBUG _DEBUG_SAY_ONLY
#else
//#define _UC20_DEBUG _DEBUG_SAY_ONLY
#define _UC20_DEBUG _DEBUG_WRITE_ONLY
//#define _UC20_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#define _WAIT_OK_TIMEOUT        1000
#define _WAIT_RESPONSE_TIMEOUT	10000

#define _PHONE_NUMERAL_NO 10

#ifndef Q_OS_OSX
#ifndef _GSM_MODULE_SERIAL_PORT 
#define _GSM_MODULE_SERIAL_PORT "/dev/ttyO1"
#endif
#endif

#define _OSX_SERIAL_PORT_DEVICE "tty.usbserial"
#define _RPI_USB_SERIAL_PORT_DEVICE     "ttyUSB"
#define _RPI_ARDUINO_SERIAL_PORT_DEVICE "ttyACM"

#ifndef _DTAC
#define _DTAC 0
#endif //_OP_DTAC_DEF

#ifndef _TRUE
#define _TRUE 1
#endif //_OP_TRUE_DEF

#ifndef _AIS
#define _AIS 2
#endif //_OP_AIS_DEF

//Please define Operator Here !!
#ifndef _OPERATOR
//#define _OPERATOR _DTAC
#define _OPERATOR _TRUE
//#define _OPERATOR _AIS
#endif // !_OPERATOR

#if _OPERATOR == _DTAC
#define _APN "www.dtac.co.th"
#define _USER ""
#define _PASS ""

#elif _OPERATOR == _TRUE //True
//#define _APN "internet"
//#define _USER "True"
//#define _PASS "true"
#define _APN "m2minternet"
#define _USER "true"
#define _PASS "true"

#elif _OPERATOR == _AIS //AIS
#define _APN "internet"
#define _USER ""
#define _PASS ""
#endif

#ifndef _HIGH
#define _HIGH   true
#endif

#ifndef _LOW
#define _LOW    false
#endif

#ifndef _GSM_PWK_PIN
#define _GSM_PWK_PIN    50
#endif

enum FILE_PATTERN_t
{
    _UFS,
    _RAM,
    _COM
};

enum URC_t
{
    _URC_USB_AT,
    _URC_USB_MODEM,
    _URC_UART1,
    _URC_UNKNOW
};

enum OPERATOR_t
{
    _OP_DTAC,
    _OP_TRUE,
    _OP_AIS
};

class HM_UC20CLASS : public QObject
{
    Q_OBJECT
public:
    HM_UC20CLASS(QObject *parent = 0);
    ~HM_UC20CLASS();

    QSerialPort *serial_port;

    bool isBegin(void) {return is_begin;}
    bool isConnected(void) {return is_connected;}
    void setPwrPin(uint16_t _pin);
	void timeoutStart(void);
	void timeoutReset(void);
	void clrSerialBuffer(void);
    bool dataAvailable(void);
    char receiveData(void);
    bool sendData(QString _data, bool _flag_ln);
    bool sendData(int _value, bool _flag_ln);
    String receiveStringUntil(String _data);
    String receiveStringUntil(char _data);
    bool begin(uint32_t _baud, uint16_t _pwr_pin);
	bool begin(uint32_t _baud);
	bool waitOK(uint32_t _time);
	bool waitOK_ndb(uint32_t _time);
	bool waitOK(uint32_t _time, bool _ack);
	bool timeoutCheck(uint32_t _time);
	bool waitToReady(uint32_t _wait_time);
	bool setPwr(bool _value);
	bool setURCPort(URC_t _port);		//QURCCFG=_port
	bool setEcho(bool _value);			//ATEn
	bool setPhoneFunc(uint8_t _value);	//CFUN=_value
	URC_t getURCPort(void);				//QURCCFG?
    String getIMEI(void);				//CGSN
    String getPhoneNum(OPERATOR_t _op);	//CUSD=1, //"*545*9#"-AIS, "*102*9#"-Dtac, "*933*9#"-True

private:
#ifdef _UC20_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _UC20_DEBUG
    HM_GPIO gpio;
    bool is_begin = false;
    bool is_connected = false;

    uint32_t serial_baud;
    uint32_t previous_time;
    uint16_t pwr_key = 0;

    void debug(QString data);
    void setPwrPinActive(void);
    bool simpleCommand(void);
    bool tryToConnect(void);
    bool connectToGSMPort(void);
    bool connectModule(void);
    void serialSend(QByteArray _byte, bool _flag_ln);

signals:

private slots:
    void slotReadyRead();

};

#endif // HM_UC20CLASS_H
