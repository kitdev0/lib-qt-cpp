#ifndef _SM_UC20_AT_FTP_H
#define _SM_UC20_AT_FTP_H

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
#ifndef _UC20_FTP_DEBUG
//#define _UC20_FTP_DEBUG _DEBUG_SAY_ONLY
//#define _UC20_FTP_DEBUG _DEBUG_WRITE_ONLY
//#define _UC20_FTP_DEBUG _DEBUG_SAY_AND_WRITE
#endif
#endif


class SM_UC20_AT_FTP_CLASS
{
public:
    SM_UC20_AT_FTP_CLASS(HM_UC20CLASS* _module);
    ~SM_UC20_AT_FTP_CLASS();
    bool setContextid(int8_t _context_ID);
    bool setAccount(String _name, String _password);
    bool setFiletype(int8_t _filetype);
    bool setTransmode(int8_t _transmode);
    bool setTimeout(uint8_t _timeout);
    bool loginToServer(String _url, uint16_t _port, bool _wait_flag);
    bool logoutFromServer(bool _wait_flag);
    bool setCurrentDir(String _path_name, bool _wait_flag);
    uint8_t getStatusFTPService(bool _wait_flag);
    bool uploadFile(String _target_file_name, QFile *_file);
private:
#ifdef _UC20_FTP_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _UC20_SIM_DEBUG
    HM_UC20CLASS *gsmModule;
    QElapsedTimer timeout;
    void debug(String data);
//    bool streamFile(String _target_name, QByteArray *_byte_data, uint16_t _start_byte);
};

#endif // SM_UC20_AT_FTP_CLASS_H
