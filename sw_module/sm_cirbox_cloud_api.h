#ifndef SM_CIRBOX_CLOUD_API_H
#define SM_CIRBOX_CLOUD_API_H

#include <QObject>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "sm_gsm_module.h"

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
#define _CLOUD_API_DEBUG _DEBUG_SAY_ONLY
#else
//#define _CLOUD_API_DEBUG _DEBUG_SAY_ONLY
//#define _CLOUD_API_DEBUG _DEBUG_WRITE_ONLY
#define _CLOUD_API_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#define _SERIAL_NUMBER_SIZE_DEF 9
#define _MESSAGE_SUCCESS    "success"

#define _API_V1_URL     "http://cirbox.cloud/api/v1/vending/"

#define _API_BUFFER_SIZE    256

#define _READ_METHOD_DELAY_TIME         500
#define _CHECK_API_BUFF_TO_SEND_TIMER   1000

#define _TABLE_NO_MAX   99
#define _API_ID_MAX     99

#define _SERIAL_NUMBER_DEFAULT_VALUE "990000002"

#ifndef _LED_ON
#define _LED_ON     _LOW
#endif

#ifndef _LED_OFF
#define _LED_OFF     _HIGH
#endif

#define _MIN_TO_MSEC(min) min*60*1000

#define _CLIENT_PING_PULLING_TIME _MIN_TO_MSEC(1) //(Min.)

class SM_CIRBOX_CLOUD_API : public QObject
{
    Q_OBJECT

public:
    explicit SM_CIRBOX_CLOUD_API(SM_GSM_MODULE *my_ethernet = 0,QObject *parent = 0);
    ~SM_CIRBOX_CLOUD_API();

    SM_GSM_MODULE *ethernet;
    bool cloud_box_ready = false;

    void setSerialNo(String _serial_no);
    bool syncTime(String _gmt);
    bool reqUpdateAPIData(void);
    bool reqClrJsonAPIData(void);
    bool setAPIData(String _table_no, String _api_id, String _data);
    uint16_t apiDataToSendAvailable(void);
    void clientPingResetTimer(void);
    inline bool getCloudBoxReady(){return cloud_box_ready;}
public slots:
    void slotReqUpdateAPI(QJsonDocument *_json_report);

private:
    SM_DEBUGCLASS *logDebug;

    String serialno = _SERIAL_NUMBER_DEFAULT_VALUE;

    bool flag_wait_to_read_response = false;
    bool flag_send_api_data = true;

    uint8_t last_api_url_set = 0;

    uint16_t api_buff = 0;
    uint16_t last_api_buff = 0;

    QTimer *check_api_buff_to_send_timer;
    QTimer *client_ping_pulling_timer;

    QJsonObject json_api_buff[_API_BUFFER_SIZE];
    QJsonObject *json_api_data;

    void debug(QString data);
    bool reportData(QJsonDocument *_json_report);
    uint16_t responseStatus(QJsonDocument *_json_response);
    String responseMessage(QJsonDocument *_json_response);
    String getMachineTime(void);
signals:
    void signalResponseAPISuccess();
    void signalResponseAPIUnsuccess();
    void signalSetLEDServer(bool _state);
    void signalConnectServerOK();
private slots:
    void slotStartToCheckAPIBuff(void);
    void slotCheckAPIBuffToSend(void);
    void slotReadResponseAPI(void);
    void slotReadResponseAPIClientPing(void);
    void slotClientPing(void);
};

#endif // SM_CIRBOX_CLOUD_API_H
