/************************************************************************/
/*
auth:   yunfei
date:   2015/02/27
email:  snowleopard331@163.com
des:    this file function is packaging and pratice glog libiary
*/
/************************************************************************/

#ifndef _GLOG_HELPER_H_
#define _GLOG_HELPER_H_

#include <glog/logging.h>
#include <glog/raw_logging.h>
#include "policy/Singleton.h"

// 将信息输出到单独的文件和 LOG(ERROR)
void signalHandle(const char* data, int size);

class GLogHelper
{
public:
    // glog config
    GLogHelper(char* program);

    // glog clear memery
    ~GLogHelper();
};

#define sLog Jovi::Singleton<GLogHelper>::Instance()

#endif//_GLOG_HELPER_H_