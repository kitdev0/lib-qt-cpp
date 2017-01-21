#ifndef SM_GSM_MODULE_H
#define SM_GSM_MODULE_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../lib-qt-cpp/hw_module/hm_uc20.cpp"
#include "sm_uc20_at_sim.h"
#include "sm_uc20_at_network.h"
#include "sm_uc20_at_packet.h"
#include "sm_uc20_at_internet.h"
#include "sm_uc20_at_http.h"
#include "sm_uc20_at_file.h"

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
#define _GSM_MODULE_DEBUG _DEBUG_SAY_ONLY
#else
//#define _GSM_MODULE_DEBUG _DEBUG_SAY_ONLY
#define _GSM_MODULE_DEBUG _DEBUG_WRITE_ONLY
//#define _GSM_MODULE_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#define _WAIT_MODULE_RAEDY_TIME 35000
#define _TRY_TO_RESET_GSM_TIME  3

#ifndef _LED_ON
#define _LED_ON     _LOW
#endif

#ifndef _LED_OFF
#define _LED_OFF     _HIGH
#endif

class SM_GSM_MODULE : public QObject
{
    Q_OBJECT
public:
    explicit SM_GSM_MODULE(QObject *parent = 0);
    ~SM_GSM_MODULE();

    HM_UC20CLASS *module = &uc20_module;
    SM_UC20_AT_SIM_CLASS *sim = new SM_UC20_AT_SIM_CLASS(&uc20_module);
    SM_UC20_AT_NETWORK_CLASS *network = new SM_UC20_AT_NETWORK_CLASS(&uc20_module);
    SM_UC20_AT_PACKET_CLASS *packet = new SM_UC20_AT_PACKET_CLASS(&uc20_module);
    SM_UC20_AT_INTERNET_CLASS *internet = new SM_UC20_AT_INTERNET_CLASS(&uc20_module);
    SM_UC20_AT_HTTP_CLASS *http = new SM_UC20_AT_HTTP_CLASS(&uc20_module);
    SM_UC20_AT_FILE_CLASS *file = new SM_UC20_AT_FILE_CLASS(&uc20_module);

    bool init(void);
    bool setInternet(void);
    bool moduleIsReady(void){return module_is_ready;}
    bool moduleCannotUse(void){return flag_gsm_module_cannot_use;}

private:    
    HM_UC20CLASS uc20_module;

#ifdef _GSM_MODULE_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _GSM_MODULE_DEBUG

    bool flag_gsm_module_cannot_use = false;
    bool module_is_ready = false;
    void debug(String data);

signals:
    void signalInternetIsOK(void);
    void signalSetLEDGsm(bool _state);

public slots:
    void slotResetGsmModule();
};

#endif // SM_GSM_MODULE_H
