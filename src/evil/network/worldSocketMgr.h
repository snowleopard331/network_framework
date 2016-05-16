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
#include "connector.h"


class ProactorRunnable;

// manages all sockets connected to peers and network threads
class WorldSocketMgr
{
public:
    friend class WorldSocket;
    friend class Evil::OperatorNew<WorldSocketMgr>;

    // start network
    int     StartNetwork();

    // stop all network threads, it will wait for running threads
    void    StopNetwork();

    // wait untill all network have "joined"
    void    Wait();

	const BSocket* getAuthBSocket() const
	{
		if (m_pAuthConnector == nullptr)
		{
			return nullptr;
		}

		return m_pAuthConnector->getSocket();
	}

	void detachAuthSocket();

private:
    WorldSocketMgr();
    virtual ~WorldSocketMgr();

    int     StartIOService();

    // handle socket accept
    int     OnSocketOpen(const boost::system::error_code &ec);

    // new boost socket obj and select a thread index
    void    OnAcceptReady();

    void    addAcceptHandler();

    void    readyReset();

	void	registToAuth();

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

    // auth
    Connector*              m_pAuthConnector;
};

#define sWorldSocketMgr Evil::Singleton<WorldSocketMgr>::Instance()

#endif//_WORLD_SOCKET_MGR_H_