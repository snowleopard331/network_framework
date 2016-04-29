/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/28
*/

#ifndef _AUTH_SOCKET_MGR_H_
#define _AUTH_SOCKET_MGR_H_

#include "policy/Singleton.h"
#include "Common.h"

class AuthSocket;

class AuthSocketMgr
{
public:

    friend Evil::OperatorNew<AuthSocketMgr>;
    typedef std::set<AuthSocket*>   SocketList;

private:

    AuthSocketMgr();
    ~AuthSocketMgr();

public:

    int startNetwork();

public:

    void OnSocketAccept(const boost::system::error_code &ec);

private:

    void addAcceptorHandler();

    void readyReset();

private:
    
    Proactor*       m_proactor;
    Acceptor*       m_acceptor;
    AuthSocket*     m_sockReady;
    SocketList      m_socketList;
};

#define sAuthSockMgr Evil::Singleton<AuthSocketMgr>::Instance()

#endif//_AUTH_SOCKET_MGR_H_