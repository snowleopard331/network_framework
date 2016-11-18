/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/02/19
*/

#ifndef _GAME_DEF_H_
#define _GAME_DEF_H_

#include "Define.h"

#ifdef DEBUG_INFO
#   define DEBUG_INFO_SOCKET
//#   define DEBUG_INFO_SOCKET_WRITE
//#   define DEBUG_INFO_CONCURRENCE_TEST
//#   define DEBUG_INFO_WRITE_AND_READ
//#   define DEBUG_INFO_STACK
#   define DEBUG_INFO_ENDIAN
//#   define DEBUG_INFO_CRYPT
//#   define DEBUG_INFO_SLEEP
//#   define DEBUG_INFO_CALLGRIND
//#   define DEBUG_INFO_DB  
//#   define DEBUG_INFO_UNIQUE_PTR
//#   define DEBUG_INFO_CONNECT
#   define DEBUG_INFO_CRUSH
#   define DEBUG_INFO_REDIS
//#   define DEBUG_INFO_STOP
#endif//DEBUG_INFO

// use in LOG
#define SEPARATOR_COMMA     ", "
#define SEPARATOR_SPACE     " "

struct PacketHeader
{
    uint16  size;
    uint16  cmd;
};

enum NetMicroDef
{
    MSG_LENGTH_MAX                          = 4096,         // tcp分包默认长度
};

enum MiscDef
{
    CONNECTOR_RECONNECT_INTERNAL_SEC        = 5,

    AUTH_SOCKET_MANAGER_LOOP_INTERNAL_SEC   = 5,
};

#endif//_GAME_DEF_H_