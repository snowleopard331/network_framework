/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#ifndef _OP_CODE_H_
#define _OP_CODE_H_

#include "Common.h"
#include "worldSession.h"

enum OpCodes
{
    MSG_NULL_ACTION                 = 0,

    MSG_AUTH_SOCKET_STARTUP,                // socket startup

    MSG_MAX,
};

class WorldPacket;

// Player state
enum SessionStatus
{
    STATUS_AUTHED   = 0,
    STATUS_LOGGEDIN,
    STATUS_TRANSFER,
    STATUS_LOGGEDIN_OR_RECENTLY_LOGGEDOUT,
    STATUS_NEVER,
    STATUS_UNHANDLER,
};

enum PacketProcessing
{
    PROCESS_INPLACE = 0,
    PROCESS_THREADUNSAFE,
    PROCESS_THREADSAFE,
};

struct OpcodeHandler
{
    const char*         name;
    SessionStatus       status;
    PacketProcessing    packetProcessing;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};

extern OpcodeHandler opcodeTable[MSG_MAX];

inline const char* LookupOpcodeName(uint16 id)
{
    if(id >= MSG_MAX)
    {
        return "Received unknown opcode, it's more than max!";
    }

    return opcodeTable[id].name;
}

#endif//_OP_CODE_H_