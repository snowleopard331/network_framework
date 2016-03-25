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
#include <boost/enable_shared_from_this.hpp>

class WorldPacket;
class WorldSession;

class WorldSocket
    : public boost::enable_shared_from_this<WorldSocket>
{
public:
    friend class WorldSocketMgr;

public:
    WorldSocket();
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
    bool close(void) const;

    // set close flag 
    // ? check close status when using BSocket every time
    void close(bool flag);

    // close the socket 
    void closeSocket(void);

    // get address of connected peer
    const std::string& getRemoteAddress(void) const;

    // send a packet on the socket
    int sendPacket(const WorldPacket& packet);

public:
    int HandleAccept();

    int Update();

private:
    int HandleInput(const boost::system::error_code &ec, size_t bytes_transferred);

#ifdef DEBUG_INFO_CONCURRENCE_TEST
    int HandleInputTest(const boost::system::error_code &ec, size_t bytes_transferred);
#endif

    int HandleOutput();

    void HandleAsyncWriteComplete(const boost::system::error_code &ec, size_t bytes_transferred);

    int HandleClose();

private:
    // helper functions for processing incoming data
    int HandleInputMissingData();
    int HandleInputHeader();
    int HandleInputPayload();

    // get proactor
    Proactor* proactor() const;

    // set proactor
    void proactor(Proactor* pPtr);

    // get socket
    BSocket* bsocket() const
    {
        return m_socket;
    }

    // set socket
    void bsocket(BSocket* sock)
    {
        if(m_socket || !sock)
        {
            // ? exception operating
        }

        m_socket = sock;
    }

    /// process one incoming packet.
    /// @param pPkt received packet ,note that you need to delete it.
    int ProcessIncoming(WorldPacket* pPkt);

    /// Flush m_PacketQueue if there are packets in it
    /// Need to be called with m_OutBufferLock lock held
    /// @return true if it wrote to the buffer ( AKA you need
    /// to mark the socket for output ).
    bool iFlushPacketQueue();

    /// Try to write WorldPacket to m_OutBuffer ,return -1 if no space
    /// Need to be called with m_OutBufferLock lock held
    int iSendPacket(const WorldPacket& pkt);

private:
    // Time in which the last ping was received
    uint32                  m_lastPingTime;

    // Keep track of over-speed pings ,to prevent ping flood
    uint32                  m_overSpeedPings;

    // Address of the remote peer
    std::string             m_address;

    // Mutex lock to protect m_Session
    LockType                m_SessionLock;

    // Session to which received packets are routed
    WorldSession*           m_Session;

    // here are stored the fragments of the received data
    WorldPacket*            m_pRecvWorldPacket;

    Buffer                  m_recvBuffer;
    Buffer                  m_header;

    // Mutex for protecting output related data
    LockType                m_OutBufferLock;
    // Buffer used for writing output.
    Buffer*                 m_outBuffer;
    // Size of the m_OutBuffer.
    size_t                  m_outBufferSize;

    /// Here are stored packets for which there was no space on m_OutBuffer,
    /// this allows not-to kick player if its buffer is overflowed.
    PacketQueue             m_PacketQueue;

    uint32                  m_seed;

    BSocket*                m_socket;

    Proactor*               m_proactor;

    bool                    m_isClose;

    char                    m_buffer[SOCKET_READ_BUFFER_SIZE];
};

#endif//_WORLD_SOCKET_H_