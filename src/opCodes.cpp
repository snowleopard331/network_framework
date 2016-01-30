/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#include "opCodes.h"
#include "worldSession.h"

OpcodeHandler opcodeTable[MSG_MAX]
{
    /*0     */  {"MSG_NULL_ACTION",                             STATUS_NEVER,       PROCESS_INPLACE,        &WorldSession::Handle_NULL              },
};