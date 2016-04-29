/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/28
*/

#include "authSocketMgr.h"
#include "authSocket.h"

#include <boost/bind.hpp>

AuthSocketMgr::AuthSocketMgr()
    : m_proactor(new Proactor)
    , m_acceptor(nullptr)
    , m_sockReady(nullptr)
{

}

AuthSocketMgr::~AuthSocketMgr()
{
    // todo: clear socketList
    if(!m_socketList.empty())
    {
        for(SocketList::iterator iter = m_socketList.begin(); iter != m_socketList.end(); ++iter)
        {
            AuthSocket* pDel = *iter;
            SafeDelete(pDel);
        }

        m_socketList.clear(); 
    }

    SafeDelete(m_sockReady);
    SafeDelete(m_acceptor);
    SafeDelete(m_proactor);
}

int AuthSocketMgr::startNetwork()
{
    uint16 port = sConfig.getIntDefault("Auth", "port", 10300);

    try
    {
        m_acceptor = new Acceptor(*m_proactor);
        EndPoint endpoint(boost::asio::ip::tcp::v4(), port);

        m_acceptor->open(endpoint.protocol());
        m_acceptor->set_option(Acceptor::reuse_address(true));
        m_acceptor->bind(endpoint);
        m_acceptor->listen();
    }
    catch(boost::system::system_error& ec)
    {
        LOG(ERROR)<<"acceptor open failed port: "<<port;
        LOG(ERROR)<<ec.what();
        return -1;
    }

    Proactor::work word(*m_proactor);
    m_proactor->run();

    LOG(INFO)<<"Auth Network starting";

    return 0;    
}

void AuthSocketMgr::addAcceptorHandler()
{
    AuthSocket* pASock = new AuthSocket();
    BSocket* pBSock = new BSocket(*m_proactor);
    pASock->bsocket(pBSock);

    m_sockReady = pASock;

    m_acceptor->async_accept(*(m_sockReady->bsocket()), 
        boost::bind(&AuthSocketMgr::OnSocketAccept, this, boost::asio::placeholders::error));
}

void AuthSocketMgr::OnSocketAccept(const boost::system::error_code &ec)
{
    if(ec)
    {
        LOG(ERROR)<<boost::system::system_error(ec).what();
        readyReset();
        return;
    }

    Evil_ASSERT(m_sockReady);
    Evil_ASSERT(m_sockReady->bsocket());

    m_sockReady->OnAccept();

    m_socketList.insert(m_sockReady);
    m_sockReady = nullptr;

    addAcceptorHandler();
}

void AuthSocketMgr::readyReset()
{
    SafeDelete(m_sockReady);

    addAcceptorHandler();
}