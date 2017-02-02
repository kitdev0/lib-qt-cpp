// sm_uc20_at_network_class.h

#ifndef SM_UC20_AT_NETWORK_H
#define SM_UC20_AT_NETWORK_H

#include "../../../../../lib-qt-cpp/hw_module/hm_uc20.h"

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
#define _UC20_NETWORK_DEBUG _DEBUG_SAY_ONLY
#else
//#define _UC20_NETWORK_DEBUG _DEBUG_SAY_ONLY
#define _UC20_NETWORK_DEBUG _DEBUG_WRITE_ONLY
//#define _UC20_NETWORK_DEBUG _DEBUG_SAY_AND_WRITE
#endif

class SM_UC20_AT_NETWORK_CLASS
{
public:
    SM_UC20_AT_NETWORK_CLASS(HM_UC20CLASS* _module);
    ~SM_UC20_AT_NETWORK_CLASS();

    bool getNetworkRegis(void);			//CREG?
    uint8_t getSignalQuality(void);		//CSQ
    String getOperator(void);			//COPS?
    bool setOperator(void);

private:
#ifdef _UC20_NETWORK_DEBUG
	SM_DEBUGCLASS *logDebug;
#endif // _UC20_SIM_DEBUG
	HM_UC20CLASS *gsmModule;
	void debug(String data);
};

#endif

