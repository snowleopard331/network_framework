/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/14
*/

#include "worldSocketMgr.h"
#include "config.h"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include <boost/asio/buffer.hpp>

#define THREAD_LOOP_INTERVAL                10000       // microsec 10000
#define CONNECTOR_SELECT_THREAD_NUM         1

class ProactorRunnable
{
public:
    ProactorRunnable()
        : m_Proactor(new Proactor)
        , m_pThread(NULL)
    {
        
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

        m_pThread = new boost::thread(boost::bind(&ProactorRunnable::threadTask, this));

        return true;
    }

    void stop()
    {
        if(m_Proactor && !m_Proactor->stopped())
        {
            // stop will cause all unfinished messages and handlers of completed but no processing be discarded immediatelly
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

    long connections()
    {
        return m_Connections;
    }

    int addSocket(WorldSocket* sock)
    {
        // mutex or recursive mutex, is deadlock ?
        boost::mutex::scoped_lock guard(m_NewSocketsLock);

        ++m_Connections;
        m_NewSockets.insert(sock);

        return 0;
    }

    Proactor* proactor() const
    {
        return m_Proactor;
    }

private:

    void addNewSockets()
    {
        boost::mutex::scoped_lock guard(m_NewSocketsLock);

        if(m_NewSockets.empty())
        {
            return;
        }

        for(SocketSet::const_iterator iter = m_NewSockets.begin(); iter != m_NewSockets.end(); ++iter)
        {
            WorldSocket* sock = *iter;

            if(sock->close())
            {
                --m_Connections;
            }
            else
            {
                m_Sockets.insert(sock);
            }
        }

        m_NewSockets.clear();
    }

    void threadTask()
    {
        LOG(INFO)<<"Network Thread Starting";

        Evil_ASSERT(m_Proactor);

        /*
            io_service::run() is blocking, if there is no handlers in io_service, 
            run() return and io_service stopped, work object could make handler count +1 
            to prevent io_service to stop
        */
        boost::asio::io_service::work work(*m_Proactor);

        Timer timer(*m_Proactor, boost::posix_time::microsec(THREAD_LOOP_INTERVAL));
        timer.async_wait(boost::bind(&ProactorRunnable::threadLoop, this, boost::ref(timer)));

        m_Proactor->run();

        LOG(INFO)<<"Network Thread Exitting";
    }

    void threadLoop(Timer& timer)
    {
        if(m_Proactor->stopped())
        {
            return;
        }

        addNewSockets();

        for(SocketSet::iterator iter = m_Sockets.begin(); iter != m_Sockets.end();)
        {
            if((*iter)->Update() == -1)
            {
                SocketSet::iterator iterTemp = iter;
                ++iter;

                (*iterTemp)->closeSocket();
                --m_Connections;
                m_Sockets.erase(iterTemp);

				// ??? instead of unique_ptr
				delete (*iterTemp);
            }
            else
            {
                ++iter;
            }
        }

        timer.expires_from_now(boost::posix_time::microsec(THREAD_LOOP_INTERVAL));
        timer.async_wait(boost::bind(&ProactorRunnable::threadLoop, this, boost::ref(timer)));
    }

private:
    typedef std::set<WorldSocket*> SocketSet;

    Proactor*                   m_Proactor;     // wrap boost io_service
    boost::thread*              m_pThread;
    boost::atomic_long          m_Connections;

    SocketSet                   m_Sockets;
    SocketSet                   m_NewSockets;

    boost::mutex                m_NewSocketsLock;
};


WorldSocketMgr::WorldSocketMgr()
    : m_NetThreads(NULL)
    , m_NetThreadsCount(0)
    , m_SockOutKBuff(-1)
    , m_SockOutUBuff(65536)
    , m_UseNoDelay(true)
    , m_Acceptor(NULL)
    , m_SoketReady(NULL)
    , m_NetThreadIndexReady(0)
    , m_pAuthConnector(NULL)
	, m_registTimer(nullptr)
{
    
}

WorldSocketMgr::~WorldSocketMgr()
{
	SafeDelete(m_registTimer);
}

int WorldSocketMgr::StartNetwork()
{
    if(StartIOService() == -1)
    {
        return -1;
    }

    return 0;
}

void WorldSocketMgr::StopNetwork()
{
    if(m_Acceptor)
    {
        m_Acceptor->close();
        SafeDelete(m_Acceptor);
    }

    if(m_NetThreadsCount != 0)
    {
        for(size_t i = 0; i < m_NetThreadsCount; ++i)
        {
            m_NetThreads[i].stop();
        }
    }
    
    Wait();
}

void WorldSocketMgr::Wait()
{
    if(m_NetThreadsCount != 0)
    {
        for(size_t i = 0; i < m_NetThreadsCount; ++i)
        {
            m_NetThreads[i].wait();
        }
    }
}

int WorldSocketMgr::OnSocketOpen(const boost::system::error_code &ec)
{
    if(ec)
    {
        LOG(ERROR)<<boost::system::system_error(ec).what();
        readyReset();
        return -1;
    }
    
    Evil_ASSERT(m_SoketReady);
    Evil_ASSERT(m_SoketReady->bsocket());

    if(m_SoketReady->HandleAccept() < 0)
    {
        // SafeDelete(m_SoketReady);
        readyReset();
        return -1;
    }

    // set some options here
    if(m_SockOutKBuff >= 0)
    {
        // SO_SNDBUF, 发送缓冲区大小
        boost::asio::socket_base::send_buffer_size option(m_SockOutKBuff);
        m_SoketReady->bsocket()->set_option(option);
    }

    // set nodelay
    if(m_UseNoDelay)
    {
        // TCP_NODELAY, 关闭Nagle算法
        boost::asio::ip::tcp::no_delay option(true);
        m_SoketReady->bsocket()->set_option(option);
    }

    m_SoketReady->m_outBufferSize = static_cast<size_t>(m_SockOutUBuff);

    if(0 == m_NetThreadIndexReady && m_NetThreadIndexReady >= m_NetThreadsCount)
    {
        readyReset();
        return -1;
    }

    if(m_NetThreads[m_NetThreadIndexReady].addSocket(m_SoketReady) < 0)
    {
        LOG(ERROR)<<"m_NetThreadIndexReady: "<<m_NetThreadIndexReady<<" add socket failed";
        SafeDelete(m_SoketReady);
    }
    else
    {
        // start async read data event
#ifdef DEBUG_INFO_CONCURRENCE_TEST
        //LOG(ERROR)<<"socket was added, thread index: "<<m_NetThreadIndexReady<<", practorAddr: "<<m_NetThreads[m_NetThreadIndexReady].proactor();
        m_SoketReady->bsocket()->async_read_some(boost::asio::buffer(m_SoketReady->m_buffer, SOCKET_READ_BUFFER_SIZE), 
            boost::bind(&WorldSocket::HandleInputTest, m_SoketReady, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
#else
        m_SoketReady->bsocket()->async_read_some(boost::asio::buffer(m_SoketReady->m_buffer, SOCKET_READ_BUFFER_SIZE), 
            boost::bind(&WorldSocket::HandleInput, m_SoketReady, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
#endif
    }

    m_NetThreadIndexReady = 0;
    m_SoketReady = NULL;

    // new handler to acceptor
    addAcceptHandler();

    return 0;
}

void WorldSocketMgr::readyReset()
{
    m_NetThreadIndexReady = 0;
    SafeDelete(m_SoketReady);

    // new handler to acceptor
    addAcceptHandler();
}

void WorldSocketMgr::addAcceptHandler()
{
    OnAcceptReady();

    Evil_ASSERT(m_NetThreadIndexReady);
    Evil_ASSERT(m_NetThreadIndexReady < m_NetThreadsCount);

    m_Acceptor->async_accept(*(m_SoketReady->bsocket()), 
        boost::bind(&WorldSocketMgr::OnSocketOpen, this, boost::asio::placeholders::error));
}

int WorldSocketMgr::StartIOService()
{
    uint16 port = sConfig.getIntDefault("Network", "Port", 10301);

    m_UseNoDelay = sConfig.getBoolDefault("Network", "TcpNoDelay", true);

    int threadNums = sConfig.getIntDefault("Network", "Threads", 1);

    if(threadNums <= 0)
    {
        LOG(ERROR)<<"Network.Threads is wrong in your config file";
        return -1;
    }

    m_NetThreadsCount = static_cast<size_t>(threadNums + 1);
    m_NetThreads = new ProactorRunnable[m_NetThreadsCount];

    /// -1 mean use default
    m_SockOutKBuff = sConfig.getIntDefault("Network", "OutKBuff", -1);
    m_SockOutUBuff = sConfig.getIntDefault("Network", "OutUBuff", 65536);

    if(m_SockOutUBuff <= 0)
    {
        LOG(ERROR)<<"Network.OutUBuff is wrong in your config file";
        return -1;
    }

    /// set acceptor
    try
    { 
        m_Acceptor = new WorldSocket::Acceptor(*(m_NetThreads[0].proactor()));
        EndPoint endpoint(boost::asio::ip::tcp::v4(), port);
        m_Acceptor->open(endpoint.protocol());
        m_Acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_Acceptor->bind(endpoint);
        m_Acceptor->listen();
    }
    catch(boost::system::system_error& ec)
    {
        LOG(ERROR)<<"acceptor open failed port: "<<port;
        LOG(ERROR)<<ec.what();
        return -1;
    }

    for(size_t i = 0; i < m_NetThreadsCount; ++i)
    {
        m_NetThreads[i].start();
    }

    addAcceptHandler();

    /// connect auth
	registToAuth();

    return 0;
}

void WorldSocketMgr::registToAuth()
{
	if(m_pAuthConnector == nullptr)
	{
		std::string authHostName = sConfig.getStringDefault("AuthInfo", "Host", "");
		Evil_ASSERT(!authHostName.empty());

		uint16 authPort = sConfig.getIntDefault("AuthInfo", "Port", 0);
		Evil_ASSERT(authPort != 0);

		m_pAuthConnector = new Connector(m_NetThreads[CONNECTOR_SELECT_THREAD_NUM].proactor(), authHostName, authPort);
	}
	else
	{
		detachAuthSocket();
	}

    // block
    if(m_pAuthConnector->syncConnect())
    {
        Evil_ASSERT(m_pAuthConnector->getSocket() != nullptr);

        WorldSocket* pConnSock = new WorldSocket();
        pConnSock->bsocket(const_cast<BSocket*>(m_pAuthConnector->getSocket()));
        Evil_ASSERT(pConnSock->HandleConnect() == 0);

        m_NetThreads[CONNECTOR_SELECT_THREAD_NUM].addSocket(pConnSock);
        LOG(INFO) << "add auth connector socket successfully";
    }
    else
    {
		if (nullptr == m_registTimer)
		{
			m_registTimer = new Timer(*(m_pAuthConnector->proactor()), boost::posix_time::seconds(CONNECTOR_RECONNECT_INTERNAL_SEC));
		}

		m_registTimer->expires_from_now(boost::posix_time::seconds(CONNECTOR_RECONNECT_INTERNAL_SEC));
		m_registTimer->async_wait(boost::bind(&WorldSocketMgr::registToAuth, this));

        // notice timer's lifetime(correct bug)
        // invoke poll/run function per thread
        m_pAuthConnector->proactor()->reset();
        m_pAuthConnector->proactor()->poll();
    }
}

void WorldSocketMgr::OnAcceptReady()
{
    size_t min = 1;

    Evil_ASSERT(m_NetThreadsCount > 1);
    // skip the Acceptor Thread
    for(size_t i = 1; i < m_NetThreadsCount; ++i)
    {
        if(m_NetThreads[i].connections() < m_NetThreads[min].connections())
        {
            min = i;
        }
    }

    WorldSocket* sock = new WorldSocket();
    BSocket* pBSock = new BSocket(*(m_NetThreads[min].proactor()));
    sock->bsocket(pBSock);

    m_SoketReady = sock;
    m_NetThreadIndexReady = min;
}

void WorldSocketMgr::detachAuthSocket()
{
	if (m_pAuthConnector == nullptr)
	{
		LOG(ERROR) << "connector member pointer is null";
		return;
	}

	m_pAuthConnector->detachSocket();
}