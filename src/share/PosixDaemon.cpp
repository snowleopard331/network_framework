/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#ifndef _POSIX_DAEMON_H_
#define _POSIX_DAEMON_H_

#include "Common.h"
#include "log/glogHelper.h"

void startDaemon(uint32 timeout = 10);

void stopDaemon();

void detachDaemon();

#endif//_POSIX_DAEMON_H_