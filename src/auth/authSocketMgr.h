/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/28
*/

#ifndef _AUTH_SOCKET_MGR_H_
#define _AUTH_SOCKET_MGR_H_

#include "policy/Singleton.h"
#include "Common.h"
#include "serverList.h"

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

    int     startNetwork();
    void    stopNetwork();

    inline Proactor* proactor() const
    {
        return m_proactor;
    }

	void addServer(std::string& addr, uint16 port)
	{
		m_serverList.add(addr, port);
	}

	void eraseServer(std::string& addr, uint16 port)
	{
		m_serverList.erase(addr, port);
	}

    // blocking
    void proactorRun()
    {
        Proactor::work work(*m_proactor);
        m_proactor->run();
    }

public:

    void OnSocketAccept(const boost::system::error_code &ec);

private:

    void addAcceptorHandler();

    void readyReset();

    void resourcesRecovery();
    void recoveryLoop(Timer& timer);

private:
    
    Proactor*       m_proactor;
    Acceptor*       m_acceptor;
    AuthSocket*     m_sockReady;
    SocketList      m_socketList;
	ServerList		m_serverList;
};

#define sAuthSockMgr Evil::Singleton<AuthSocketMgr>::Instance()

#endif//_AUTH_SOCKET_MGR_H_