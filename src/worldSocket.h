/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#ifndef _WORLD_SOCKET_H_
#define _WORLD_SOCKET_H_

#include "Common.h"
#include "buffer.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/mutex.hpp>

class WorldPacket;

class WorldSocket
{
public:
    friend WorldSocketMgr;

public:
    WorldSocket(Proactor* proactor);
    ~WorldSocket();

public:
    // declare the acceptor for this class
    typedef boost::asio::ip::tcp::acceptor          Acceptor;
    // Mutex type used for various synchronizations.
    typedef boost::mutex                            LockType;
    typedef boost::unique_lock<LockType>            GuardType;

    typedef std::list<WorldPacket*>                 PacketQueue;

    // packet queue

    // check if socket is closed
    bool isClose(void) const;

    // close the socket 
    void closeSocket(void);

    // get address of connected peer
    const std::string& getRemoteAddress(void) const;

    // send a packet on the socket
    int sendPacket(const WorldPacket& packet);

    // get proactor
    Proactor* proactor() const;

    // set proactor
    void proactor(Proactor* pPtr);

    // get socket
    BSocket* getSocket() const;

public:
    int HandleAccept();

private:
    // helper functions for processing incoming data
    int handleInputHeader(void);
    int handleInputPayload(void);
    int handleInputMissingData(void);

private:
    // Time in which the last ping was received
    uint32                  m_lastPingTime;

    // Keep track of over-speed pings ,to prevent ping flood
    uint32                  m_overSpeedPings;

    // Address of the remote peer
    std::string             m_address;

    // here are stored the fragments of the received data
    WorldPacket*            m_pRecvWorldPacket;

    Buffer                  m_recvBuffer;
    Buffer                  m_header;

    // Buffer used for writing output.
    Buffer*                 m_outBuffer;
    // Size of the m_OutBuffer.
    size_t                  m_outBufferSize;
    // True if the socket is registered with the proactor for output
    bool                    m_outActive;

    uint32                  m_seed;

    BSocket                 m_socket;

    Proactor*               m_proactor;
};

#endif//_WORLD_SOCKET_H_