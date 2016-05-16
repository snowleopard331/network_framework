/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/05/03
*/

#include "serverList.h"

ServerList::ServerList()
{

}

ServerList::~ServerList()
{
    
}

void ServerList::add(std::string& addr, uint16 port)
{
    ServerInfo* pInfo = new ServerInfo;

    pInfo->address = addr;
    pInfo->port = port;
    pInfo->registTime = time(0);

    m_serverList.push_back(pInfo);

    LOG(INFO) << "add server host " << addr<<SEPARATOR_SPACE << "port " <<port;
	LOG(INFO) << "server list size " << m_serverList.size();
}

void ServerList::erase(std::string& addr, uint16 port)
{
    if(m_serverList.empty())
    {
        return;
    }

    for(ServerListType::iterator iter = m_serverList.begin(); iter != m_serverList.end();)
    {
        ServerListType::iterator itTemp = iter++;

        if((*itTemp)->address == addr && port == (*itTemp)->port)
        {
            SafeDelete(*itTemp);
            m_serverList.erase(itTemp);

            LOG(INFO)<<"erase server, IP : "<<addr<<SEPARATOR_COMMA<<"port : "<<port;
            break;
        }
    }

	LOG(INFO) << "server list size " << m_serverList.size();
}

void ServerList::clear()
{
    if(m_serverList.empty())
    {
        return;
    }

    for(ServerListType::iterator iter = m_serverList.begin(); iter != m_serverList.end();)
    {
        LOG(INFO)<<"erase server, IP : "<<(*iter)->address<<SEPARATOR_COMMA<<"port : "<<(*iter)->port;

        SafeDelete(*iter);
        iter = m_serverList.erase(iter);
    }

    return;
}