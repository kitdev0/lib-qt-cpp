#ifndef SM_CIRBOX_CLOUD_PROTOCOL_H
#define SM_CIRBOX_CLOUD_PROTOCOL_H

#include <QObject>
#include <QTimer>
#include "sm_debug.h"
#include "sm_cirbox_cloud_api.h"
#include "sm_cb_protocol_cmd.h"

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
#define _CLOUD_BOX_PROTOCOL_DEBUG _DEBUG_SAY_ONLY
#else
//#define _CLOUD_BOX_PROTOCOL_DEBUG _DEBUG_SAY_ONLY
#define _CLOUD_BOX_PROTOCOL_DEBUG _DEBUG_WRITE_ONLY
//#define _CLOUD_BOX_PROTOCOL_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#ifndef _CB_SERIAL_PORT_NAME
//#define _CB_SERIAL_PORT_NAME "/dev/ttyO4"
#endif

#define _CB_SERIAL_BAUD 115200

#ifndef _OSX_SERIAL_PORT_DEVICE
#define _OSX_SERIAL_PORT_DEVICE "tty.usbserial"
#endif
//
#define _CB_CMD_TRUP "TRUP"

#define _CLIENT_TIMEOUT_TIME 60000
#define _CHECK_COMPORT_TIMER 5000

class SM_CIRBOX_CLOUD_PROTOCOL : public QObject
{
    Q_OBJECT
public:
    explicit SM_CIRBOX_CLOUD_PROTOCOL(SM_CIRBOX_CLOUD_API *api_port = 0, QObject *parent = 0);
    ~SM_CIRBOX_CLOUD_PROTOCOL();

    bool begin(uint32_t _baud);

private:
#ifdef _CLOUD_BOX_PROTOCOL_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _CLOUD_BOX_PROTOCOL_DEBUG
    SM_CIRBOX_CLOUD_API *api;
    SM_CB_PROTOCOL_CMD cmd;
    QSerialPort *cb_serial_port;
    QTimer *client_online_time;
    QTimer *check_comport_timer;

    bool is_begin = false;
    bool flag_client_is_connected = false;
    bool flag_first_scanning = true;
    bool flag_first_serial_not_found = true;
    int8_t machine_client_connect_ok = -1;

    int16_t last_usb_err = -1;
    uint32_t cb_serial_baud;
    int16_t last_mid = -1;
    int16_t current_mid = -2;

    void debug(QString data);
    void sendData(String _data);
    bool tryToConnect(void);

signals:
    void signalReadCBProtocol(String);
    void signalReturnOK(void);
    void signalReturnSuccess(void);
    void signalReturnError(void);
    void signalSetDataValue(String _str);
    void signalGetDataValue(String _str);
    void signalSetLEDClient(bool _state);
    void signalReportDataToCloud(QJsonDocument *_json_doc);
    void signalClientISConnect();

public slots:
    void slotResetClientTimeoutTime();

private slots:
    void slotReadSerialPort(void);
    void slotReadCBProtocol(String);
    void slotReturnMID(int16_t _mid);
    void slotReturnOK(void);
    void slotReturnError(void);
    void slotGetDataValue(String _str);
    void slotSetDataValue(String _str);
    void slotReturnSuccess();
    void slotReturnUnsuccess();
    void slotCheckClientTimeout();
    void slotCheckComport();
    void slotReturnReady();
    void slotReturnBusy();
    void slotSerialError(QSerialPort::SerialPortError _error);
};

#endif // SM_CIRBOX_CLOUD_PROTOCOL_H
