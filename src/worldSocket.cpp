/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#include "worldSocket.h"
#include "worldSocketMgr.h"
#include "worldPacket.h"

WorldSocket::WorldSocket(Proactor* proactor)
{

}

WorldSocket::~WorldSocket()
{
    if(m_outBuffer)
    {
        SafeDelete(m_outBuffer);
    }
}

int WorldSocket::HandleAccept()
{
    // prevent double call to this func
    if(m_outBuffer)
    {
        return -1;
    }

    m_outActive = true;

    // hook for the manager
    if(sWorldSocketMgr->onSocketOpen(this) == -1)
    {
        return -1;
    }

    //  new buffer
    m_outBuffer = new Buffer(m_outBufferSize);

    m_address = m_socket.remote_endpoint().address().to_string();

    WorldPacket packet(MSG_AUTH_SOCKET_STARTUP, 4);
    packet << m_seed;

    if(sendPacket(packet) == -1)
    {
        return -1;
    }

    return 0;
}