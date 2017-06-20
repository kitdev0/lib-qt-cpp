#ifndef SM_CIRBOX_CLOUD_API_H
#define SM_CIRBOX_CLOUD_API_H

#define _FIRMWARE_VERSION 3.03

#include <QObject>
#include <QTimer>
#include <QDateTime>
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
#ifndef _CLOUD_API_DEBUG
#define _CLOUD_API_DEBUG _DEBUG_SAY_ONLY
#endif
#else
#ifndef _CLOUD_API_DEBUG
//#define _CLOUD_API_DEBUG _DEBUG_SAY_ONLY
#define _CLOUD_API_DEBUG _DEBUG_WRITE_ONLY
//#define _CLOUD_API_DEBUG _DEBUG_SAY_AND_WRITE
#endif
#endif

#define _SERIAL_NUMBER_SIZE_DEF 8
#define _MESSAGE_SUCCESS        "success"
#define _MESSAGE_DEACTIVATED    "deactivated"

#define _API_V1_VENDING_URL "/api/v1/vending/"
#define _API_V1_LOG_URL     "/api/v1/log?"
#define _CIRBOX_CLOUD_URL   "http://cirbox.cloud"
//#define _CIRBOX_CLOUD_URL   "http://cirbox-cloud-asia.appspot.com"
//#define _API_V1_URL         "http://cirbox-cloud-asia.appspot.com/api/v1/vending/"
#define _CIRBOX_HOST        "cirbox.cloud"

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

#define _CLIENT_PING_PULLING_TIME _MIN_TO_MSEC(0.5) //(Min.)

#define _CCID_LEN_MAX   32

#define _SERVICE_SC "SC"
#define _SERVICE_MC "MC"

#define _CHECK_TO_POST_LOG_TIME 5000

#define _TRY_TO_OPEN_CONNECT 5
#define _TRY_TO_SET_URL     5
#define _TRY_TO_GET_METHOD  5
#define _APP_LOOP_TIME      2000

class SM_CIRBOX_CLOUD_API : public QObject
{
    Q_OBJECT

public:
    explicit SM_CIRBOX_CLOUD_API(SM_GSM_MODULE *my_ethernet = 0, QObject *parent = 0);
    ~SM_CIRBOX_CLOUD_API();

    SM_GSM_MODULE *ethernet;
    bool connect_server_ready = false;

    void setSerialNo(String _serial_no);
    bool reqUpdateAPIData(void);
    bool reqClrJsonAPIData(void);
    bool setAPIData(String _table_no, String _api_id, String _data);
    static bool STATIC_BOOL_WAIT_TO_REBOOT;
    uint16_t apiDataToSendAvailable(void);
    void clientPingResetTimer(void);
    inline bool getConnectServerReady(){return connect_server_ready;}
    void configSystem(QString _machine_type);
    void setMachineType(QString _machine_type);
public slots:
    void slotReqUpdateAPI(QJsonDocument *_json_report);
    void slotCreatRunAppScript(QString _service_id);
//    bool slotUploadLogFile(String _date);
private:
    SM_DEBUGCLASS *logDebug;

    String serialno = _SERIAL_NUMBER_DEFAULT_VALUE;
    String last_time_syntime = "";

    bool flag_wait_to_read_response = false;
    bool flag_send_api_data = true;
    bool flag_set_led_off_all = false;
    bool flag_report_data = false;
    bool flag_synctime_success = false;
    bool flag_to_post_log = false;
    bool flag_post_log_unsuccess = false;
    bool flag_to_ping = false;

    bool flag_deactiveted = false;
    bool flag_api_response_incorrect = false;

    uint8_t last_api_url_set = 0;
    uint8_t try_to_slot_synctime_cnt = 0;
    uint8_t try_post_report_again = 0;
    uint8_t try_post_log_again = 0;
    uint8_t try_ping_again = 0;
    uint8_t try_to_read_response = 0;
    uint8_t response_api_incorrect_cnt = 0;
    uint8_t try_to_ping_to_server = 0;
    uint8_t try_to_client_ping = 0;

    uint16_t api_buff = 0;
    uint16_t last_api_buff = 0;

    uint32_t try_synctime_cnt = 0;
    uint32_t try_post_report_cnt = 0;
    uint32_t try_post_log_cnt = 0;
    uint32_t try_ping_cnt = 0;
    uint32_t try_client_ping_cnt = 0;

    QTimer *check_api_buff_to_send_timer;
    QTimer *client_ping_pulling_timer;
    QTimer *systime_timer;
    QTimer *check_flag_post_log_timer;

    QJsonObject json_api_buff[_API_BUFFER_SIZE];
    QJsonObject *json_api_data;

    QString machine_type = "";
    QString date_get_log = "";
    QString post_log_error_info = "";

    void debug(QString data);
    uint16_t responseStatus(QJsonDocument *_json_response);
    String responseMessage(QJsonDocument *_json_response);
    uint8_t responseCmd(QJsonDocument *_json_response);
    QString responseDeviceID(QJsonDocument *_json_response);
    String getMachineTime(void);
    void checkToExecuteCmd(uint8_t _cmd);
    void cmdRemove_RunAppScript();
    void cmdChmodX_RunAppScript();
    void cmdRemove_interfaceFile();
    void checkResponseIDToChangeService(QString _response_id);
    QString responseCmdData(QJsonDocument *_json_response);
//    bool connectInternet();
//    bool httpSetURL(String _url);
//    bool httpGetMethod(String _url);
    bool syncTime(String _gmt, bool _flag_sync_again);
    int8_t reportData(QJsonDocument *_json_report);
    bool clientPing();
    int8_t postLog();
    bool postLog(String _str);
    void resetPingTime();
    void responseAPIIncorrect();
signals:
//    void signalResponseAPISuccess();
//    void signalResponseAPIUnsuccess();
    void signalSetLEDServer(bool _state);
    void signalStartLoopApp();
    void signalOffLEDAll();
//    void signalResetClientTimeoutTime();
    void signalResetInternet();
    void signalTryToInitModule(uint8_t);

private slots:
//    void slotStartToCheckAPIBuff(void);
//    void slotStopToCheckAPIBuff();
//    void slotCheckAPIBuffToSend(void);
    void slotReadResponseAPI(void);
    void slotReadResponseAPIClientPing(void);
//    void slotClientPing(void);
    void slotSyncTime();
//    void slotGetTimeFromServer();
    void slotCloudBoxShutdown();
    void slotCloudBoxReboot();
    void slotUnmountSDCard();
//    void slotPostLog();
    void slotTryToInitModule(uint8_t _try_cnt);
    void slotAppLoop();
    void slotSetFlagPing();
    void slotWaitResponsePing();
    void slotTryPingToServer();
};

#endif // SM_CIRBOX_CLOUD_API_H
