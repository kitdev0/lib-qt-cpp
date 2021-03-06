// sm_uc20_at_internet_class.h

#ifndef SM_UC20_AT_INTERNET_H
#define SM_UC20_AT_INTERNET_H

#include <QObject>
#include "../../../../lib-qt-cpp/hw_module/hm_uc20.h"

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
#define _UC20_INTERNET_DEBUG _DEBUG_SAY_ONLY
#else
//#define _UC20_INTERNET_DEBUG _DEBUG_SAY_ONLY
#define _UC20_INTERNET_DEBUG _DEBUG_WRITE_ONLY
//#define _UC20_INTERNET_DEBUG _DEBUG_SAY_AND_WRITE
#endif

class SM_UC20_AT_INTERNET_CLASS : public QObject
{
    Q_OBJECT

public:
    SM_UC20_AT_INTERNET_CLASS(HM_UC20CLASS* _module, QObject *parent = 0);
	~SM_UC20_AT_INTERNET_CLASS();

	bool configure(String apn, String user, String password);
	bool configure(void);
	bool connect(void);
	bool disConnect(void);
	bool isConnect(void);
    String getIP(void);
    bool resetConnecting(void);


private:
#ifdef _UC20_INTERNET_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _UC20_INTERNET_DEBUG
    HM_UC20CLASS *gsmModule;
    void debug(String data);

signals:
    void signalResetGsmModule();
};


#endif

