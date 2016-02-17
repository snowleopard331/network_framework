/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#ifndef _MASTER_H_
#define _MASTER_H_

#include "Common.h"

class Master
{
public:
    Master();

    ~Master();

    int Run();

    static volatile uint32 m_MasterLoopCounter;

};

#define sMaster Jovi::Singleton<Master>::Instance()

#endif//_MASTER_H_