/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/14
*/

#include "worldSocketMgr.h"
#include "worldSocket.h"
#include "config.h"

int WorldSocketMgr::startNetwork(uint16 port, std::string& address)
{
    if(startIOService(port, address.c_str()))
    {
        return -1;
    }

    return 0;
}

int WorldSocketMgr::onSocketOpen(WorldSocket* sock)
{
    // set some options here
    if(m_SockOutKBuff >= 0)
    {
        boost::asio::ip::tcp::
        sock->getSocket()->set_option()
    }

    // set nodelay
    if(m_UseNoDelay)
    {
        boost::asio::ip::tcp::no_delay option(true);
        sock->getSocket()->set_option(option);
    }

    sock->m_outBufferSize = static_cast<size_t>(m_SockOutUBuff);

    // 
}


int WorldSocketMgr::startIOService(uint16 port, const char* address)
{
    m_UseNoDelay = sConfig.getBoolDefault("Network", "TcpNoDelay", true);

    int threadNums = sConfig.getIntDefault("Network", "Threads", 1);

    if(threadNums <= 0)
    {
        LOG(ERROR)<<"Network.Threads is wrong in your config file";
        return -1;
    }

    m_NetThreadsNum = static_cast<size_t>(threadNums + 1);
    m_NetThreads = new ProactorRunnable[m_NetThreadsNum];

    // -1 mean use default
    m_SockOutKBuff = sConfig.getIntDefault("Network", "OutKBuff", -1);
    m_SockOutUBuff = sConfig.getIntDefault("Network", "OutUBuff", 65536);

    if(m_SockOutUBuff <= 0)
    {
        LOG(ERROR)<<"Network.OutUBuff is wrong in your config file";
        return -1;
    }

    m_Acceptor = new WorldSocket::Acceptor(*(m_NetThreads[0].getProactor()));

    // ?
    // boost::asio::ip::tcp::v4();
    boost::asio::ip::address addr;
    addr.from_string(address);
    EndPoint endpoint(addr, port);

    // set acceptor
    m_Acceptor->open(endpoint.protocol());
    m_Acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    m_Acceptor->bind(endpoint);
    m_Acceptor->listen();

    for(size_t i = 0; i < m_NetThreadsNum; ++i)
    {
        m_NetThreads[i].start();
    }

    return 0;
}