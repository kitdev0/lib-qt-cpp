// sm_uc20_at_file_class.h

#ifndef _SM_UC20_AT_FILE_H
#define _SM_UC20_AT_FILE_H

//#include "../../../../../lib-qt-cpp/hw_module/hm_uc20.h"
#include "/Users/kitdev/Google Drive/CirboxDesign/lib-qt-cpp/hw_module/hm_uc20.h"

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
#ifndef _UC20_FILE_DEBUG
#define _UC20_FILE_DEBUG _DEBUG_SAY_ONLY
#endif
#else
#ifndef _UC20_FILE_DEBUG
//#define _UC20_FILE_DEBUG _DEBUG_SAY_ONLY
//#define _UC20_FILE_DEBUG _DEBUG_WRITE_ONLY
//#define _UC20_FILE_DEBUG _DEBUG_SAY_AND_WRITE
#endif
#endif

class SM_UC20_AT_FILE_CLASS
{
public:
    SM_UC20_AT_FILE_CLASS(HM_UC20CLASS* _module);
	~SM_UC20_AT_FILE_CLASS();

	void begin();
	void write(char data);
	void list(FILE_PATTERN_t pattern);
    void (*listOutput)(QString data);
    void readFile(FILE_PATTERN_t pattern, QString file_name);
	void (*dataOutput)(char data);
	bool available();
    bool del(FILE_PATTERN_t pattern, QString fn);
	bool close(int handle);
	bool beginWrite(int handle, int size);
	bool waitFinish();
	bool seek(int handle, long start_at);
	bool seekAtStart(int handle);
    int open(FILE_PATTERN_t pattern, QString fn);
	int read(int handle, int buf_size, char *buf);
    QString readLine();
	long getFreeSpace(FILE_PATTERN_t pattern);
    long list(FILE_PATTERN_t pattern, QString fn);
	long getSpace(FILE_PATTERN_t pattern);

private:
#ifdef _UC20_FILE_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _UC20_SIM_DEBUG

    HM_UC20CLASS *gsmModule;
    //static QString _data = "";
    void debug(QString data);
    long space(FILE_PATTERN_t pattern, unsigned char mode);

};

#endif

