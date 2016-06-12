/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/05/09
*/

#ifndef _CONNETCOR_H_
#define _CONNETCOR_H_

#include "Common.h"
#include <boost/function.hpp>

//enum ConnectorStates
//{
//    CONNETTOR_STATES_CLOSED,
//    CONNETTOR_STATES_OPEN,
//};

enum TcpOption
{
    OPTION_NO_DELAY         = 0x0001,
};

class Connector
{
public:
    explicit Connector(Proactor* proactor, std::string& hostname, uint16 port);
    ~Connector();

    typedef boost::function<void (bool)>         CallbackFunc;

public:
    void asyncConnect(CallbackFunc func);

    bool syncConnect();

    const BSocket* getSocket() const
    {
        return m_socket;
    }

    void setOption(uint option)
    {
        if(0 == option)
        {
            LOG(WARNING)<<"set tcp option is zero";
            return;
        }

        if(option & OPTION_NO_DELAY)
        {
            boost::asio::ip::tcp::no_delay option(true);
            m_socket->set_option(option);
        }

        return;
    }

	/*
		call this function when you wouldn't to delete Connector object 
		but m_socket was destroyed and must call this, if not will become wild pointer
	*/
	void detachSocket()
	{
		m_socket = nullptr;
	}

    Proactor* proactor() const
    {
        return m_proactor;
    }



private:
    void HandleConnect(const boost::system::error_code& ec);
    void HandleConnect(const boost::system::error_code& ec, CallbackFunc func);

private:
    std::string         m_hostName;
    uint16              m_port;
    Proactor*           m_proactor;
    BSocket*            m_socket;
};

#endif//_CONNETCOR_H_