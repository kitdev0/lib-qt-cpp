#include "sm_uc20_at_file_v101.h"

void func_null(String) {}
void func_null(char) {}
bool available_ = true;

SM_UC20_AT_FILE_CLASS::SM_UC20_AT_FILE_CLASS(HM_UC20CLASS* _module)
{
#ifdef _UC20_FILE_DEBUG
	logDebug = new SM_DEBUGCLASS("UC20_FILE");
#endif
    gsmModule = _module;
}

SM_UC20_AT_FILE_CLASS::~SM_UC20_AT_FILE_CLASS()
{
#ifdef _UC20_FILE_DEBUG
	delete logDebug;
#endif
	delete gsmModule;
}

void SM_UC20_AT_FILE_CLASS::debug(String data)
{
#ifdef _UC20_FILE_DEBUG
#if _UC20_FILE_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _UC20_FILE_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _UC20_FILE_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

//public
void SM_UC20_AT_FILE_CLASS::begin()
{
    debug("begin");
	listOutput = func_null;
	dataOutput = func_null;
}

void SM_UC20_AT_FILE_CLASS::list(FILE_PATTERN_t pattern)
{
	//AT+QFLST="*"
    //AT+QFLST="RAM:*
    if (pattern == _UFS){
        if(!gsmModule->sendData("AT+QFLST=\"*\"", 1))
            return;
    }
    if (pattern == _RAM){
        if(!gsmModule->sendData("AT+QFLST=\"RAM:*\"", 1))
            return;
    }

	available_ = true;

    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _req = gsmModule->serial_port->readLine();
            //debug("read data >> " + _req);

            if (_req.indexOf("OK") != -1)
            {
                debug("OK");
                return;
            }
            else if (_req.indexOf("ERROR") != -1)
            {
                debug("ERROR!!");
                return;
            }
            (*listOutput)(_req);
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return;
        }
    }
}

void SM_UC20_AT_FILE_CLASS::readFile(FILE_PATTERN_t pattern, String file_name)
{
    debug("readFile");
	int handle = open(pattern, file_name);
	if (handle != -1)
	{
		const int buf_size = 128;
		char buf[buf_size];
		int size_ = 0;
		seekAtStart(handle);
		size_ = read(handle, buf_size, buf);
		while (size_ != -1)
		{
			for (int i = 0; i<size_; i++)
			{
				//Serial.write(buf[i]);
				(*dataOutput)(buf[i]);
			}
			size_ = read(handle, buf_size, buf);
		}
	}
	close(handle);
}

bool SM_UC20_AT_FILE_CLASS::available()
{
	return(available_);
}

bool SM_UC20_AT_FILE_CLASS::del(FILE_PATTERN_t pattern, String fn)
{
    debug("del");
    if(!gsmModule->sendData("AT+QFDEL=\"", 0))
        return 0;

	if (pattern == _UFS)
	{
        if(!gsmModule->sendData(fn, 0))
            return 0;
	}

	if (pattern == _RAM)
	{
        if(!gsmModule->sendData("RAM:", 0))
            return 0;
        if(!gsmModule->sendData(fn, 0))
            return 0;
	}
    if(!gsmModule->sendData("\"",1))
        return 0;
    return gsmModule->waitOK(_WAIT_OK_TIMEOUT);
}

bool SM_UC20_AT_FILE_CLASS::close(int handle)
{
    debug("close");
    if(!gsmModule->sendData("AT+QFCLOSE=",0))
        return 0;
    if(!gsmModule->sendData(handle,0))
        return 0;
    if(!gsmModule->sendData("",1))
        return 0;
    return gsmModule->waitOK(_WAIT_OK_TIMEOUT);
}

bool SM_UC20_AT_FILE_CLASS::beginWrite(int handle, int size)
{
	//AT+QFWRITE=0,10
    debug("beginWrite");
    if(!gsmModule->sendData("AT+QFWRITE=",0))
        return 0;
    if(!gsmModule->sendData(handle,0))
        return 0;
    if(!gsmModule->sendData(",",0))
        return 0;
    if(!gsmModule->sendData(size,0))
        return 0;
    if(!gsmModule->sendData("\r\n",1))
        return 0;

    while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _req = gsmModule->serial_port->readLine();
            //debug("read data >> " + _req);

            if (_req.indexOf("CONNECT") != -1)
            {
                debug("CONNECTED");
                return 1;
            }
            else if (_req.indexOf("ERROR") != -1)
            {
                debug("ERROR!!");
                return 0;
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return 0;
        }
    }
}

bool SM_UC20_AT_FILE_CLASS::waitFinish()
{
	return(gsmModule->waitOK(10000));
}

bool SM_UC20_AT_FILE_CLASS::seekAtStart(int handle)
{
	return(seek(handle, 0));
}

bool SM_UC20_AT_FILE_CLASS::seek(int handle, long start_at)
{
	//AT+QFSEEK=0,0,0
    debug("seek");
    if(!gsmModule->sendData("AT+QFSEEK=",0))
        return 0;
    if(!gsmModule->sendData(handle,0))
        return 0;
    if(!gsmModule->sendData(",",0))
        return 0;
    if(!gsmModule->sendData(start_at,0))
        return 0;
    if(!gsmModule->sendData(",",0))
        return 0;
    if(!gsmModule->sendData("0",1))
        return 0;
	return(gsmModule->waitOK(_WAIT_OK_TIMEOUT));
}

int SM_UC20_AT_FILE_CLASS::read(int handle, int buf_size, char *buf)
{
	//AT+QFREAD=0,10
	int size = 0;
	int cnt = 0;
    debug("read");
    if(!gsmModule->sendData("AT+QFREAD=",0))
        return 0;
    if(!gsmModule->sendData(handle,0))
        return 0;
    if(!gsmModule->sendData(",",0))
        return 0;
    if(!gsmModule->sendData(buf_size,0))
        return 0;
    if(!gsmModule->sendData("",1))
        return 0;

	while (1)
    {
        if(gsmModule->serial_port->canReadLine())
        {
            String _req = gsmModule->serial_port->readLine();
            //debug("read data >> " + _req);
            if (_req.indexOf("CONNECT") != -1){
                char index1 = _req.indexOf(" ");
                String str = _req.mid(index1 + 1);
                //debug("size = "+str);
                size = str.toInt();
                debug("CONNECTED");
                break;
            }
            else if (_req.indexOf("ERROR") != -1) {
                debug("Invalid data!!");
                return(-1);
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return (-2);
        }
	}
	

	while (1)
	{
        if (gsmModule->dataAvailable())
		{
            char c = gsmModule->receiveData();
			buf[cnt] = c;
			//Serial.write(c);
			cnt++;
			if (cnt >= size)
			{
				break;
			}
		}
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return (-2);
        }
	}
	gsmModule->waitOK(_WAIT_OK_TIMEOUT);
	return(size);
}

int SM_UC20_AT_FILE_CLASS::open(FILE_PATTERN_t pattern, String fn)
{
    debug("open");
    if(!gsmModule->sendData("AT+QFOPEN=\"",0))
        return 0;
	if (pattern == _UFS)
	{
        if(!gsmModule->sendData(fn,0))
            return 0;
	}
	if (pattern == _RAM)
	{
        if(!gsmModule->sendData("RAM:",0))
            return 0;
        if(!gsmModule->sendData(fn,0))
            return 0;
	}
    if(!gsmModule->sendData("\",0",1))
        return 0;
	
	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _req = gsmModule->serial_port->readLine();
            //debug("read data >> " + _req);
            if (_req.indexOf("+QFOPEN:") != -1)
            {
                char index1 = _req.indexOf(" ");
                String str = _req.mid(index1 + 1);
                //Serial.print("OK xxx");
                //Serial.print(str);
                gsmModule->waitOK(_WAIT_OK_TIMEOUT);
                return(str.toInt());
            }
            else if (_req.indexOf("ERROR") != -1)
            {
                debug("ERROR!!");
                return(-1);
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return (-2);
        }
	}

}

String SM_UC20_AT_FILE_CLASS::readLine()
{
    debug("readLine");
    String _req = "";

    if(gsmModule->serial_port->canReadLine())
    {
        _req = gsmModule->serial_port->readLine();
        //debug("read data >> " + _req);
        if (_req.indexOf("OK") != -1)
        {
            available_ = false;
        }
        (*listOutput)(_req);
    }
    return(_req);
}

long SM_UC20_AT_FILE_CLASS::list(FILE_PATTERN_t pattern, String fn)
{
    debug("list");
	if (pattern == _UFS)
	{
        if(!gsmModule->sendData("AT+QFLST=\"",0))
            return 0;
        if(!gsmModule->sendData(fn,0))
            return 0;
        if(!gsmModule->sendData("\"",1))
            return 0;
	}
	if (pattern == _RAM)
	{
		//AT+QFLST="RAM:1"
        if(!gsmModule->sendData("AT+QFLST=\"RAM:",0))
            return 0;
        if(!gsmModule->sendData(fn,0))
            return 0;
        if(!gsmModule->sendData("\"",1))
            return 0;
	}
	//+QFLST: "RAM:1",0

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _req = gsmModule->serial_port->readLine();
            //debug("read data >> " + _req);
            if (_req.indexOf("+QFLST:") != -1)
            {
                char index1 = _req.indexOf(",");
                String str_l = _req.mid(index1 + 1);
                gsmModule->waitOK(_WAIT_OK_TIMEOUT);
                return(str_l.toLong());
            }
            else if (_req.indexOf("ERROR") != -1)
            {
                debug("ERROR!!");
                return(-1);
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return (-2);
        }
	}
}

long SM_UC20_AT_FILE_CLASS::getSpace(FILE_PATTERN_t pattern)
{
	return(space(pattern, 0));
}

long SM_UC20_AT_FILE_CLASS::getFreeSpace(FILE_PATTERN_t pattern)
{
	return(space(pattern, 1));
}

long SM_UC20_AT_FILE_CLASS::space(FILE_PATTERN_t pattern, unsigned char mode)//Mode 0 = All Space,1 = Free Space
{
    debug("space");
    long _l = 0;
    if(!gsmModule->sendData("AT+QFLDS=",0))
        return 0;
    if(pattern == _UFS){
        if(!gsmModule->sendData("\"UFS\"",1))
            return 0;
    }
    else if(pattern == _RAM){
        if(!gsmModule->sendData("\"RAM\"",1))
            return 0;
    }
    else if (pattern == _COM){
        if(!gsmModule->sendData("\"COM\"",1))
            return 0;
    }

	while (1)
	{
        if(gsmModule->serial_port->canReadLine())
        {
            String _req = gsmModule->serial_port->readLine();
            //debug("read data >> " + _req);
            if (_req.indexOf("+QFLDS:") != -1)
            {
                char index1, index2;
                if (mode)//FreeSpace
                {
                    index1 = _req.indexOf(" ");
                    index2 = _req.indexOf(",");
                    String str_l = _req.mid(index1 + 1, index2 - index1);
                    _l = str_l.toLong();

                }
                else//Space
                {
                    char index1 = _req.indexOf(",");
                    String str_l = _req.mid(index1 + 1);
                    _l = str_l.toLong();
                }
                gsmModule->waitOK(_WAIT_OK_TIMEOUT);
                return(_l);
            }
        }
        else if(!gsmModule->serial_port->waitForReadyRead(_WAIT_OK_TIMEOUT + 5000)){
            debug("Response timeout!!");
            return (-2);
        }
        //
	}
}
