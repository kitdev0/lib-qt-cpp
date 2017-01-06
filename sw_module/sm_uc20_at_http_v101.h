// sm_uc20_at_http_class.h

#ifndef SM_UC20_AT_HTTP_V101_H
#define SM_UC20_AT_HTTP_V101_H

#include "hw_module/hm_uc20_v101.h"

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
#define _UC20_HTTP_DEBUG _DEBUG_SAY_ONLY
#else
//#define _UC20_HTTP_DEBUG _DEBUG_SAY_ONLY
#define _UC20_HTTP_DEBUG _DEBUG_WRITE_ONLY
//#define _UC20_HTTP_DEBUG _DEBUG_SAY_AND_WRITE
#endif

#define _HTTP_STATUS_CONTINUE           100
#define _HTTP_STATUS_SWITCH_PROTOCOLS   101

//Http status : Success 2xx
#define _HTTP_STATUS_OK                 200
#define _HTTP_STATUS_CREATED            201
#define _HTTP_STATUS_ACCEPTED           202
#define _HTTP_STATUS_NON_AUTHORITATIVE  203
#define _HTTP_STATUS_NO_CONTENT         204
#define _HTTP_STATUS_RESET_CONTENT      205
#define _HTTP_STATUS_PARTICAL_CONTENT   206

//Http status : Redirect 3xx
#define _HTTP_STATUS_MULTIPLE_CHOICES   300
#define _HTTP_STATUS_MOVED_PERMANENTLY  301
#define _HTTP_STATUS_FOUND              302
#define _HTTP_STATUS_SEE_OTHER          303
#define _HTTP_STATUS_NOT_MODIFIED       304
#define _HTTP_STATUS_USE_PROXY          305
#define _HTTP_STATUS_UNUSED             306
#define _HTTP_STATUS_TEMPORARY_REDIRECT 307

//Http status : Client Error 4xx
#define _HTTP_STATUS_BAD_REQUEST        400
#define _HTTP_STATUS_UNAUTHURIZED       401
#define _HTTP_STATUS_PAYMENT_REQUIRED   402
#define _HTTP_STATUS_FORBIDDEN          403
#define _HTTP_STATUS_NOT_FOUND          404
#define _HTTP_STATUS_METHOD_NOT_ALLOWED 405
#define _HTTP_STATUS_NOT_ACCEPTABLE     406
#define _HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED  407
#define _HTTP_STATUS_REQUEST_TIMEOUT                408
#define _HTTP_STATUS_CONFLICT                       409
#define _HTTP_STATUS_GONE                           410
#define _HTTP_STATUS_LENGTH_REQUIRED                411
#define _HTTP_STATUS_PRECONDITION_FAILED            412
#define _HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE       413
#define _HTTP_STATUS_REQUEST_URI_TOO_LONG           414
#define _HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE         415
#define _HTTP_STATUS_REQUEST_RANGE_NOT_SATISFIABLE  416
#define _HTTP_STATUS_EXPECTATION_FAILED             417

//Http status : Server Error 5xx
#define _HTTP_STATUS_INTERNAL_SERVER_ERROR          500
#define _HTTP_STATUS_NOT_IMPLEMENTED                501
#define _HTTP_STATUS_BAD_GATEWAY                    502
#define _HTTP_STATUS_SERVICE_UNAVAILABLE            503
#define _HTTP_STATUS_GATEWAY_TIMEOUT                504
#define _HTTP_STATUS_VERSION_NOT_SUPPORTED          505


class SM_UC20_AT_HTTP_CLASS
{
public:
    SM_UC20_AT_HTTP_CLASS(HM_UC20CLASS* _module);
	~SM_UC20_AT_HTTP_CLASS();

	void read(void);
	void readReturn(void);
	//String checkReadReturnData(void);

	bool flag_set_url = false;
	bool flag_send_url = false;
	bool flag_get_method = false;
	bool flag_read_method = false;
	bool flag_ready_to_set_url = false;
	bool flag_ready_to_send_url = false;
	bool flag_ready_to_get_method = false;
	bool flag_ready_to_read_method = false;
	bool flag_get_method_error = false;
	bool flag_response_data_is_ok = false;
	bool flag_read_method_connect = false;
	bool flag_read_method_error = false;
	//bool flag_gsm_module_is_ready = true;

    int16_t server_response_value;

	void resetFlag(void);
	void timeoutStart(void);

	bool setContextid(int8_t context_ID);
	bool setRequestHeader(bool _value);
	bool setResponseHeader(bool _value);
	bool setURL(String url);
	bool setURL(String _url, bool _need_return);
	bool sendURL(bool _need_return);
	bool saveResponseToMemory(FILE_PATTERN_t pattern, String Filename);

	bool checkResponseTimeout(void);

	int16_t getMethod(bool _wait_flag);
	int16_t checkGetRequestReturn(void);
	int16_t postMethod(void);
	int16_t postMethod(String data);

	String readMethod(bool _wait_flag);
	String response_data;

private:
#ifdef _UC20_HTTP_DEBUG
    SM_DEBUGCLASS *logDebug;
#endif // _UC20_SIM_DEBUG
    HM_UC20CLASS *gsmModule;
    String  return_data;
    String URL_BUFF;
    void debug(String data);
};


#endif

