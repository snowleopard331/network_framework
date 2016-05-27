/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/05/09
*/

#include "connector.h"
#include <boost/bind.hpp>

Connector::Connector(Proactor* proactor, std::string& hostname, uint16 port)
    : m_hostName(hostname)
    , m_port(port)
    , m_proactor(proactor)
    , m_socket(nullptr)
{
    
}

Connector::~Connector()
{
    SafeDelete(m_socket);
}

bool Connector::syncConnect()
{
    if(m_proactor == nullptr)
    {
        ELOG(ERROR)<<"input proactor is illegal, proactor point is nullptr";
        return false;
    }

    if(m_proactor->stopped())
    {
        ELOG(ERROR)<<"input proactor is illegal, proactor have stopped";
        return false;
    }

    Evil_ASSERT(m_socket == nullptr);

    m_socket = new BSocket(*m_proactor);

    boost::asio::ip::address addr;
    addr.from_string(m_hostName);
    EndPoint endpoint(addr, m_port);

    LOG(INFO)<<"connect host : "<<m_hostName<<SEPARATOR_COMMA<<"port : "<<m_port;

    boost::system::error_code ec;

	// sync
    m_socket->connect(endpoint, ec);

	HandleConnect(ec);

    return (ec ? false : true);
}

void Connector::asyncConnect()
{
    if(m_proactor == nullptr)
    {
        ELOG(ERROR)<<"input proactor is illegal, proactor point is nullptr";
        return;
    }
    
    if(m_proactor->stopped())
    {
        ELOG(ERROR)<<"input proactor is illegal, proactor have stopped";
        return;
    }
    
    Evil_ASSERT(m_socket == nullptr);

    m_socket = new BSocket(*m_proactor);

    boost::asio::ip::address addr;
    addr.from_string(m_hostName);
    EndPoint endpoint(addr, m_port);

    LOG(INFO)<<"connect host : "<<m_hostName<<SEPARATOR_COMMA<<"port : "<<m_port;

    m_socket->async_connect(endpoint, boost::bind(&Connector::HandleConnect, this, boost::asio::placeholders::error));
}

void Connector::HandleConnect(const boost::system::error_code& ec)
{
    if(ec)
    {
        ELOG(ERROR)<<boost::system::system_error(ec).what();
		SafeDelete(m_socket);

        return;
    }

    LOG(INFO) << "connect hostname " << m_hostName << SEPARATOR_SPACE
        << "ip " << m_port << SEPARATOR_SPACE
        << "successfully";

    return;
}