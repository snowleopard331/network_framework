/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/14
*/

#ifndef _WORLD_SOCKET_MGR_H_
#define _WORLD_SOCKET_MGR_H_

#include "Common.h"
#include "worldSocket.h"
#include "policy/Singleton.h"

class ProactorRunnable;

// manages all sockets connected to peers and network threads
class WorldSocketMgr
{
public:
    friend class WorldSocket;
    friend class Jovi::OperatorNew<WorldSocketMgr>;

    // Start network, listen at address:port .
    int     StartNetwork(uint16 port, std::string& address);

    // stop all network threads, it will wait for running threads
    void    StopNetwork();

    // wait untill all network have "joined"
    void    Wait();

private:
    WorldSocketMgr();
    virtual ~WorldSocketMgr();

    int     StartIOService(uint16 port, const char* address);

    // handle socket accept
    int     OnSocketOpen();
    void    OnAcceptReady();
    void    AddAcceptHandler();

private:
    ProactorRunnable*       m_NetThreads;
    size_t                  m_NetThreadsCount;

    int                     m_SockOutKBuff;
    int                     m_SockOutUBuff;
    bool                    m_UseNoDelay;

    std::string             m_addr;
    uint16                  m_port;

    WorldSocket::Acceptor*  m_Acceptor;

    WorldSocket*            m_SoketReady;
    size_t                  m_NetThreadIndexReady;
};

#define sWorldSocketMgr Jovi::Singleton<WorldSocketMgr>::Instance()

#endif//_WORLD_SOCKET_MGR_H_