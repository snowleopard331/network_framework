/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/05/09
*/

#ifndef _CODE_H_
#define _CODE_H_

enum OpCodes
{
    MSG_NULL_ACTION                 = 0,

    // s -> s
    MSG_AUTH_EVIL_REGIST            = 1,
    MSG_AUTH_EVIL_UNREGIST,

    
    MSG_EVIL_AUTH_SOCKET_STARTUP,   // s -> c

    MSG_MAX,
};

#endif//_CODE_H_
