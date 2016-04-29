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

    int run();

private:

    bool _startDB();

private:

    DatabaseType    m_loginDatabase;
    bool            m_stopEvent;
};

#define sMaster Evil::Singleton<Master>::Instance()

#endif//_AUTH_MASTER_H_