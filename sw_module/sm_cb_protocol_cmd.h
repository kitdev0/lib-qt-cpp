#ifndef SM_CB_PROTOCOL_CMD_H
#define SM_CB_PROTOCOL_CMD_H

#include <stdint.h>
#include <stdbool.h>
#include <QString>

typedef QString String;

class SM_CB_PROTOCOL_CMD
{
public:
    SM_CB_PROTOCOL_CMD(){;}
//    const String TRUP = "TRUP";
//    const String SRUP = "SRUP";
//    const String TRCLRALL = "TRCLRALL";
//    const String SRCLRALL = "SRCLRALL";
//    const String TRAPI = "TRAPI";
//    const String SRAPI = "SRAPI";
    const String UPDATE = "UP";
    const String CLEAR = "CLR";
    const String SET = "SET";
    const String GET = "GET";
    const String STATE = "STATE";
    const String READY = "READY";
};

#endif // SM_CB_PROTOCOL_CMD_V101_H
