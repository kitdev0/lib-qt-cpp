#ifndef SM_GSM_MODULE_V101_H
#define SM_GSM_MODULE_V101_H

#include "hw_module/hm_uc20_v101.h"
#include "sm_uc20_at_sim_v101.h"
#include "sm_uc20_at_network_v101.h"
#include "sm_uc20_at_packet_v101.h"
#include "sm_uc20_at_internet_v102.h"
#include "sm_uc20_at_http_v101.h"
#include "sm_uc20_at_file_v101.h"

class SM_GSM_MODULECLASS
{
private:
    //QSerialPort *serial_port;
    HM_UC20CLASS uc20_module;

public:
    SM_GSM_MODULECLASS() {}
	~SM_GSM_MODULECLASS() {}
    void begin() { ; }
    HM_UC20CLASS *module = &uc20_module;
    SM_UC20_AT_SIM_CLASS *sim = new SM_UC20_AT_SIM_CLASS(&uc20_module);
    SM_UC20_AT_NETWORK_CLASS *network = new SM_UC20_AT_NETWORK_CLASS(&uc20_module);
    SM_UC20_AT_PACKET_CLASS *packet = new SM_UC20_AT_PACKET_CLASS(&uc20_module);
    SM_UC20_AT_INTERNET_CLASS *internet = new SM_UC20_AT_INTERNET_CLASS(&uc20_module);
    SM_UC20_AT_HTTP_CLASS *http = new SM_UC20_AT_HTTP_CLASS(&uc20_module);
    SM_UC20_AT_FILE_CLASS *file = new SM_UC20_AT_FILE_CLASS(&uc20_module);
};

#endif
