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

    boost::system::error_code ec;
    //std::string port = numToStr(m_port);

    boost::asio::ip::tcp::resolver resolver(*proactor());
    boost::asio::ip::tcp::resolver::query query(m_hostName, numToStr(m_port));
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

    boost::asio::connect(*m_socket, iter, ec);

	HandleConnect(ec);

    return (ec ? false : true);
}

void Connector::asyncConnect(CallbackFunc func)
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

    // std::string port = numToStr(m_port);

    boost::asio::ip::tcp::resolver resolver(*proactor());
    boost::asio::ip::tcp::resolver::query query(m_hostName, numToStr(m_port));
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

    boost::asio::async_connect(*m_socket, iter, boost::bind(&Connector::HandleConnect, this, boost::asio::placeholders::error, func));
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

void Connector::HandleConnect(const boost::system::error_code& ec, CallbackFunc func)
{
    /// callback func
    func(ec ? false : true);

    if(ec)
    {
        ELOG(ERROR)<<"error code : "<<boost::system::system_error(ec).code().value();
        ELOG(ERROR)<<boost::system::system_error(ec).what();
        m_socket->close();
        SafeDelete(m_socket);
        return;
    }

    LOG(INFO) << "connect hostname " << m_hostName << SEPARATOR_SPACE
        << "ip " << m_port << SEPARATOR_SPACE
        << "successfully";

    return;
}