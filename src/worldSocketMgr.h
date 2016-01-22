/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/14
*/

#ifndef _WORLD_SOCKET_MGR_H_
#define _WORLD_SOCKET_MGR_H_

#include "Common.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>


class WorldSocket;

class ProactorRunnable
{
    Proactor*                   m_Proactor;     // wrap boost io_service
    boost::thread*              m_pThread;
    boost::atomic_long          m_Connections;

public:
    ProactorRunnable()
        : m_Proactor(0)
        , m_pThread(NULL)
    {
        m_Proactor = new Proactor;
    }

    ~ProactorRunnable()
    {
        stop();
        wait();

        SafeDelete(m_pThread);
        SafeDelete(m_Proactor);
    }

    bool start()
    {
        if(m_pThread)
        {
            return false;
        }

        boost::function0<void> threadTaskFun = boost::bind(&ProactorRunnable::threadTask, this);
        m_pThread = new boost::thread(threadTaskFun);

        return true;
    }

    void stop()
    {
        if(m_Proactor && !m_Proactor->stopped())
        {
            m_Proactor->stop();
        }
    }

    void wait()
    {
        if(m_pThread)
        {
            m_pThread->join();
        }
    }

    long getConnetions()
    {
        return m_Connections;
    }

    int addSocket(WorldSocket* sock)
    {

    }

    Proactor* getProactor() const
    {
        return m_Proactor;
    }

private:

    void threadTask();
};

// manages all sockets connected to peers and network threads
class WorldSocketMgr
{


public:
    friend class WorldSocket;

    // Start network, listen at address:port .
    int startNetwork(uint16 port, std::string& address);

    // stop all network threads, it will wait for running threads
    void stopNetwork();

    // wait untill all network have "joined"
    void wait();

    // make this class singleton
    static WorldSocketMgr* getInstance();

private:
    int onSocketOpen(WorldSocket* sock);
    int startIOService(uint16 port, const char* address);

    WorldSocketMgr();
    virtual ~WorldSocketMgr();

private:
    ProactorRunnable*       m_NetThreads;
    size_t                  m_NetThreadsNum;

    int                     m_SockOutKBuff;
    int                     m_SockOutUBuff;
    bool                    m_UseNoDelay;

    std::string             m_addr;
    uint16                  m_port;

    WorldSocket::Acceptor*  m_Acceptor;
};

#define sWorldSocketMgr WorldSocketMgr::getInstance()

#endif//_WORLD_SOCKET_MGR_H_