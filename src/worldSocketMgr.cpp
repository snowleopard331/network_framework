/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/14
*/

#include "worldSocketMgr.h"
#include "config.h"

#ifdef DEBUG_INFO_CRYPT
#include "authCrypt.h"
#endif

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include <boost/asio/buffer.hpp>

#define THREAD_LOOP_INTERVAL    10000       // microsec 10000

class ProactorRunnable
{
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
        // ? Is safe delete socket first
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

        Jovi_ASSERT(m_Proactor);

        /*
            io_service::run() is blocking, if there is no handlers in io_service, 
            run() return and io_service stopped, work object could make handler count +1 
            to prevent io_service to stop
        */
        boost::asio::io_service::work work(*m_Proactor);

        boost::asio::deadline_timer timer(*m_Proactor, boost::posix_time::microsec(THREAD_LOOP_INTERVAL));
        timer.async_wait(boost::bind(&ProactorRunnable::threadLoop, this, boost::ref(timer)));

        m_Proactor->run();

        LOG(INFO)<<"Network Thread Exitting";
    }

    void threadLoop(boost::asio::deadline_timer &timer)
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
{

}

WorldSocketMgr::~WorldSocketMgr()
{

}

int WorldSocketMgr::StartNetwork(uint16 port, std::string& address)
{
    if(StartIOService(port, address.c_str()) == -1)
    {
        return -1;
    }
    srand(time(0));
#ifdef DEBUG_INFO_CRYPT
    std::string plainText = "hello cryptopp";
    std::string cipherText;
    //CryptRSA crypt;
    //std::cout<<"plainText : "<<plainText<<std::endl;
    //crypt.EncryptSend(plainText, cipherText);
    //std::cout<<"cipherText : "<<cipherText<<std::endl;
    //plainText.clear();
    //crypt.DecryptRecv(plainText, cipherText);
    //std::cout<<"plainText : "<<plainText<<std::endl;
    for(int i = 0; i < 1; ++i)
    {
        std::cout<<"Index : "<<i<<std::endl;
        plainText += (rand()%26 + 'a');
        std::cout<<"plainText : "<<plainText<<std::endl;
        crypt.EncryptSend(plainText, cipherText);
        plainText.clear();
        std::cout<<"cipherText : "<<cipherText<<std::endl;
        crypt.DecryptRecv(plainText, cipherText);
        cipherText.clear();
        std::cout<<"plainText : "<<plainText<<std::endl;

        std::cout<<std::endl;
    }

#endif

    return 0;
}

void WorldSocketMgr::StopNetwork()
{
    if(m_Acceptor)
    {
        m_Acceptor->close();
    }

    if(m_NetThreadsCount != 0)
    {
        for(size_t i = 0; i < m_NetThreadsCount; ++i)
        {
            m_NetThreads[i].stop();
        }
    }
    // ？ 可能没有意义 stop执行时候run就已经退出了
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
        ReadyReset();
        return -1;
    }
    
    Jovi_ASSERT(m_SoketReady);
    Jovi_ASSERT(m_SoketReady->bsocket());
    
#ifdef DEBUG_INFO_SOCKET_WRITE
    LOG(ERROR)<<"OnSocketOpen, socketAddr: "<<m_SoketReady<<", "
        <<"bsocketAddr: "<<m_SoketReady->bsocket();
#endif

    if(m_SoketReady->HandleAccept() < 0)
    {
        // SafeDelete(m_SoketReady);
        ReadyReset();
        return -1;
    }

    // set some options here
    if(m_SockOutKBuff >= 0)
    {
        // SO_SNDBUF
        boost::asio::socket_base::send_buffer_size option(m_SockOutKBuff);
        m_SoketReady->bsocket()->set_option(option);
    }

    // set nodelay
    if(m_UseNoDelay)
    {
        // TCP_NODELAY
        boost::asio::ip::tcp::no_delay option(true);
        m_SoketReady->bsocket()->set_option(option);
    }

    m_SoketReady->m_outBufferSize = static_cast<size_t>(m_SockOutUBuff);

    if(0 == m_NetThreadIndexReady && m_NetThreadIndexReady >= m_NetThreadsCount)
    {
        ReadyReset();
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
        LOG(ERROR)<<"socket was added, thread index: "<<m_NetThreadIndexReady<<", practorAddr: "<<m_NetThreads[m_NetThreadIndexReady].proactor();
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
    AddAcceptHandler();

    return 0;
}

void WorldSocketMgr::ReadyReset()
{
    m_NetThreadIndexReady = 0;
    SafeDelete(m_SoketReady);

    // new handler to acceptor
    AddAcceptHandler();
}

void WorldSocketMgr::AddAcceptHandler()
{
    OnAcceptReady();

    Jovi_ASSERT(m_NetThreadIndexReady);
    Jovi_ASSERT(m_NetThreadIndexReady < m_NetThreadsCount);

    m_Acceptor->async_accept(*(m_SoketReady->bsocket()), 
        boost::bind(&WorldSocketMgr::OnSocketOpen, this, boost::asio::placeholders::error));
}

int WorldSocketMgr::StartIOService(uint16 port, const char* address)
{
    m_UseNoDelay = sConfig.getBoolDefault("Network", "TcpNoDelay", true);

    int threadNums = sConfig.getIntDefault("Network", "Threads", 1);

    if(threadNums <= 0)
    {
        LOG(ERROR)<<"Network.Threads is wrong in your config file";
        return -1;
    }

    m_NetThreadsCount = static_cast<size_t>(threadNums + 1);
    m_NetThreads = new ProactorRunnable[m_NetThreadsCount];

    // -1 mean use default
    m_SockOutKBuff = sConfig.getIntDefault("Network", "OutKBuff", -1);
    m_SockOutUBuff = sConfig.getIntDefault("Network", "OutUBuff", 65536);

    if(m_SockOutUBuff <= 0)
    {
        LOG(ERROR)<<"Network.OutUBuff is wrong in your config file";
        return -1;
    }

    m_Acceptor = new WorldSocket::Acceptor(*(m_NetThreads[0].proactor()));

    // boost::asio::ip::tcp::v4();
    boost::asio::ip::address addr;
    addr.from_string(address);
    EndPoint endpoint(addr, port);

    // set acceptor
    try
    {
        m_Acceptor->open(endpoint.protocol());
        m_Acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_Acceptor->bind(endpoint);
        m_Acceptor->listen();
    }
    catch(boost::system::system_error& ec)
    {
        LOG(ERROR)<<"local ip: "<<address<<", port: "<<port;
        LOG(ERROR)<<ec.what();
        return -1;
    }

    for(size_t i = 0; i < m_NetThreadsCount; ++i)
    {
        m_NetThreads[i].start();
    }

    AddAcceptHandler();

    return 0;
}

void WorldSocketMgr::OnAcceptReady()
{
    size_t min = 1;

    Jovi_ASSERT(m_NetThreadsCount > 1);
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

