/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/22
*/

#ifndef _AUTH_MASTER_H_
#define _AUTH_MASTER_H_

#include "policy/Singleton.h"
#include "database/DatabaseEnv.h"

class AuthSocket;

class Master
{
public:

    friend Evil::OperatorNew<Master>;

private:

    Master();

    ~Master();

public:

    int     run();

    void    stop();


private:

    bool    _startDB();

    void    _masterLoop(Timer &timer);

private:

    DatabaseType    m_loginDatabase;
    bool            m_stopEvent;

    uint            m_loopCounter;
    uint            m_numLoops;             // maximum counter for next ping
};

#define sMaster Evil::Singleton<Master>::Instance()

#endif//_AUTH_MASTER_H_