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

class Connector
{
public:
    explicit Connector(Proactor* proactor, std::string& hostname, uint16 port);
    ~Connector();

public:
    void asyncConnect();

    bool syncConnect();

    const BSocket* getSocket() const
    {
        return m_socket;
    }

	/*
		call this function when you don't want to delete Connector object 
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

private:
    std::string         m_hostName;
    uint16              m_port;
    Proactor*           m_proactor;
    BSocket*            m_socket;
};

#endif//_CONNETCOR_H_