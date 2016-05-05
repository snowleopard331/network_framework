/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/29
*/

#ifndef _AUTH_CODE_H_
#define _AUTH_CODE_H_

enum AuthCmd
{
    CMD_AUTH_NULL                   = 0,
    CMD_AUTH_EVIL_REGIST,
    CMD_AUTH_EVIL_UNREGIST,

    CMD_AUTH_MAX,       // keep this in the last
};

#endif//_AUTH_CODE_H_