#ifndef SM_CLIENT_MQTT_API_H
#define SM_CLIENT_MQTT_API_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../../../../../lib-qt-cpp/qmqtt/qmqtt.h"
#include "../../../../../lib-qt-cpp/sw_module/sm_debug.h"


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
#define _SM_CLIENT_MQTT_API_DEBUG _DEBUG_SAY_ONLY
#else
//#define _SM_CLIENT_MQTT_API_DEBUG _DEBUG_SAY_ONLY
#define _SM_CLIENT_MQTT_API_DEBUG _DEBUG_WRITE_ONLY
//#define _SM_CLIENT_MQTT_API_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#define _API_MAX 99
#define _CLIENT_NO_MAX  32

#define _MESSAGE_SUCCESS        "success"
#define _MESSAGE_UNSUCCESS      "unsuccess"

#define _MQTT_ID    "999"
#define _MQTT_NAME  "root"
#define _MQTT_PASS  "cirbox2012"

#define _CLIENT_TIMEOUT_TIME 30000

#ifndef _LED_ON
#define _LED_ON     false
#endif

#ifndef _LED_OFF
#define _LED_OFF     true
#endif

class SM_CLIENT_MQTT_API : public QObject
{
    Q_OBJECT
public:
    explicit SM_CLIENT_MQTT_API(QObject *parent = 0);
    ~SM_CLIENT_MQTT_API();
    void init(void);
    void setClientIdList(QJsonDocument *_json_doc);
    void initMachineStatus(void);
signals:
    void signalCheckMqttAPI(QString _data);
    void signalCheckReportData(QJsonDocument *_json_doc);
    void signalCheckStatusData(QJsonDocument *_json_doc);
    void signalReportDataToCloud(QJsonDocument *_json_doc);
    void signalSetLEDClient(bool _state);
public slots:

private:
    SM_DEBUGCLASS *logDebug;
    QMQTT::Client *mqttClient;
    QTimer *client_online_time;

    struct machine_t {
        bool c_state; //true = connect, false = disconnect
        String c_status;
        String id;
        String m_status;
        String e_code;
    };

    machine_t machine_status[_CLIENT_NO_MAX];

    uint8_t client_no = 0;
    void debug(String data);
    void returnMessage(String _id,String _message);

private slots:
    void slotMqttConnected();
    void slotMqttDisconnected();
    void slotMqttReceived(const QMQTT::Message &_message);
    void slotCheckReportData(QJsonDocument *_json_doc);
    void slotCheckStatusData(QJsonDocument *_json_doc);
    void slotCheckClientTimeout();
};

#endif // SM_CLIENT_MQTT_API_H
