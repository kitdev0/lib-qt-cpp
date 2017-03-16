#include "sm_uc20_at_ftp.h"

SM_UC20_AT_FTP_CLASS::SM_UC20_AT_FTP_CLASS(HM_UC20CLASS* _module)
{
#ifdef _UC20_FTP_DEBUG
    logDebug = new SM_DEBUGCLASS("UC20_FTP");
#endif
    gsmModule = _module;
}

SM_UC20_AT_FTP_CLASS::~SM_UC20_AT_FTP_CLASS()
{
#ifdef _UC20_FTP_DEBUG
    delete logDebug;
#endif
    delete gsmModule;
}

void SM_UC20_AT_FTP_CLASS::debug(String data)
{
#ifdef _UC20_FTP_DEBUG
#if _UC20_FTP_DEBUG == _DEBUG_SAY_ONLY
    logDebug->sayln(data);
#elif _UC20_FTP_DEBUG == _DEBUG_WRITE_ONLY
    logDebug->writeLog(data);
#elif _UC20_FTP_DEBUG == _DEBUG_SAY_AND_WRITE
    logDebug->sayAndWriteLog(data);
#endif
#endif
}

bool SM_UC20_AT_FTP_CLASS::setContextid(int8_t _context_ID)
{
    String _data = "AT+QFTPCFG=\"contextid\",";
    String _debug = "set Context-ID >> ";
    debug(_debug + String::number(_context_ID,10));

    if(!gsmModule->sendData(_data + String::number(_context_ID,10),1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_FTP_CLASS::setAccount(String _name,String _password)
{
    String _data = "AT+QFTPCFG=\"account\",";
    String _debug = "set Account >> ";
    debug(_debug + "User:" + _name + ", Pass:" + _password);
    _data += "\"" + _name + "\",";
    _data += "\"" + _password + "\"";
    debug("setAccount - " + _data);

    if(!gsmModule->sendData(_data,1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_FTP_CLASS::setFiletype(int8_t _filetype)
{
    String _data = "AT+QFTPCFG=\"filetype\",";
    String _debug = "set File type >> ";
    debug(_debug + String::number(_filetype,10));

    if(!gsmModule->sendData(_data + String::number(_filetype,10),1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_FTP_CLASS::setTransmode(int8_t _transmode)
{
    String _data = "AT+QFTPCFG=\"transmode\",";
    String _debug = "set Transmode >> ";
    debug(_debug + String::number(_transmode,10));

    if(!gsmModule->sendData(_data + String::number(_transmode,10),1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_FTP_CLASS::setTimeout(uint8_t _timeout)
{
    String _data = "AT+QFTPCFG=\"rsptimeout\",";
    String _debug = "set Timeout >> ";
    debug(_debug + String::number(_timeout,10) + " Sec.");

    if(_timeout > 180 || _timeout < 20){
        debug("Warning !! >> Value not in rang 20 - 180.");
        return 0;
    }

    if(!gsmModule->sendData(_data + String::number(_timeout,10),1))
        return 0;
    return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

bool SM_UC20_AT_FTP_CLASS::loginToServer(String _url, uint16_t _port, bool _wait_flag)
{
    String _data = "AT+QFTPOPEN=";
    String _debug = "Login to FTP Server >> ";
    _data += "\"" + _url + "\",";
    _data += String::number(_port,10);
    debug(_debug + "URL:" + _url + ", PORT:" + String::number(_port,10));
//    debug("loginToFTPServer - " + _data);

    if(!gsmModule->sendData(_data,1))
        return 0;

    while (_wait_flag)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _data1 = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_data1.indexOf("+QFTPOPEN:") != -1)
            {
                debug(_debug + "Success");
                return 1;
            }
            else if (_data1.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _data1);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT + 5000)){
            debug("getMethod >> Response timeout!!");
            return -2;
        }
    }
    return 0;
}

bool SM_UC20_AT_FTP_CLASS::logoutFromServer(bool _wait_flag)
{
    String _data = "AT+QFTPCLOSE";
    String _debug = "Logout from FTP Server";
    debug(_debug);

    if(!gsmModule->sendData(_data,1))
        return 0;

    while (_wait_flag)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _data1 = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_data1.indexOf("+QFTPCLOSE:") != -1)
            {
                debug(_debug + " >> Success");
                return 1;
            }
            else if (_data1.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _data1);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT + 5000)){
            debug("getMethod >> Response timeout!!");
            return -2;
        }
    }
    return 0;
}

bool SM_UC20_AT_FTP_CLASS::setCurrentDir(String _path_name, bool _wait_flag)
{
    String _data = "AT+QFTPCWD=";
    String _debug = "Set current dir. >> ";
    _data += "\"" + _path_name + "\"";
    debug(_debug + _path_name);
    debug("setCurrentDir - " + _data);

    if(!gsmModule->sendData(_data,1))
        return 0;

    while (_wait_flag)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _data1 = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_data1.indexOf("+QFTPCWD:") != -1)
            {
                return 1;
            }
            else if (_data1.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _data);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT + 5000)){
            debug("getMethod >> Response timeout!!");
            return -2;
        }
    }
    return 0;
}

uint8_t SM_UC20_AT_FTP_CLASS::getStatusFTPService(bool _wait_flag)
{
    String _data = "AT+QFTPSTAT";
    String _debug = "Get status FTP Service >> ";

    if(!gsmModule->sendData(_data,1))
        return 0;

    while (_wait_flag)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _data1 = gsmModule->serial_port->readLine();
            //debug("read data >> " + _str);
            if (_data1.indexOf("+QFTPSTAT:") != -1)
            {
                char index1 = _data1.indexOf(",");
                uint8_t _stat = _data1.mid(index1 + 1, 1).toInt();
                switch (_stat) {
                case 0:
                    debug(_debug + "Opening an FTP service");
                    break;
                case 1:
                    debug(_debug + "The FTP service is opened and idle");
                    break;
                case 2:
                    debug(_debug + "Transferring data with FTP server");
                    break;
                case 3:
                    debug(_debug + "Closing the FTP service");
                    break;
                case 4:
                    debug(_debug + "The FTP service is closed");
                    break;
                default:
                    debug("Warning >> Status code not found.");
                    break;
                }
                return _stat;
            }
            else if (_data1.indexOf("ERROR") != -1)
            {
                debug("ERROR >> " + _data1);
                return -1;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_RESPONSE_TIMEOUT + 5000)){
            debug("getMethod >> Response timeout!!");
            return -2;
        }
    }
    return 0;
}
