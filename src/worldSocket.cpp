/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#include "worldSocket.h"
#include "worldSocketMgr.h"
#include "worldPacket.h"

#include "boost/bind.hpp"

struct PacketHeader
{
    uint16  size;
    uint16  cmd;
};

WorldSocket::WorldSocket()
    : m_lastPingTime(0)
    , m_overSpeedPings(0)
    , m_Session(NULL)
    , m_pRecvWorldPacket(NULL)
    , m_recvBuffer()
    , m_header(sizeof(PacketHeader))
    , m_outBuffer()
    , m_outBufferSize(65536)
    , m_seed(static_cast<uint32>(rand()))
    , m_socket(NULL)
    , m_proactor(NULL)
    , m_isClose(true)
{
    memset(m_buffer, 0, SOCKET_READ_BUFFER_SIZE);
}

WorldSocket::~WorldSocket()
{
    if(m_outBuffer)
    {
        SafeDelete(m_outBuffer);
    }

    if(m_socket)
    {
        SafeDelete(m_socket);
    }
}

int WorldSocket::HandleAccept()
{
    // prevent double call to this func
    if(m_outBuffer)
    {
        return -1;
    }

    close(false);

    //  new buffer
    m_outBuffer = new Buffer(m_outBufferSize);

    m_address = m_socket->remote_endpoint().address().to_string();

    WorldPacket packet(MSG_AUTH_SOCKET_STARTUP, 4);
    packet << m_seed;

    if(sendPacket(packet) == -1)
    {
        return -1;
    }

    return 0;
}

bool WorldSocket::close() const
{
    return m_isClose;
}

void WorldSocket::close(bool flag)
{
    m_isClose = flag;
}

int WorldSocket::Update()
{
    if(m_isClose)
    {
        return -1;
    }

    if(m_outBuffer->length() == 0)
    {
        return 0;
    }

    return HandleOutput();
}

int WorldSocket::HandleInput()
{
    if(m_isClose)
    {
        return -1;
    }

    
    switch(HandleInputMissingData())
    {
    case -1:
        {
            // ? why do this
            return Update();
        }
    case -2:
        {
            return -1;
        }
    case 0:
        {
            LOG(ERROR)<<"WorldSocket::handle_input: Peer has closed connection";
            return -1;
        }
    case 1:
        {
            return 1;
        }
    default:
        return Update();
    }

    return  -1;
}

int WorldSocket::HandleInputMissingData()
{
    uint size = strlen(m_buffer);

    if(0 == size)
    {
        return 0;
    }
    
    Buffer message_block(m_buffer, size);

    message_block.wr_ptr(size);

    while(message_block.length() > 0)
    {
        if(m_header.space() > 0)
        {
            const size_t toHeaderSize = (message_block.length() > m_header.space() ? m_header.space() : message_block.length());
            m_header.copy(message_block.rd_ptr(), toHeaderSize);
            message_block.rd_ptr(toHeaderSize);
        }

        if(m_header.space() > 0)
        {
            // Couldn't receive the whole header this time
            Jovi_ASSERT(message_block.length() == 0);
            return -1;// EWOULDBLOCK
        }

        // received a nice new header
        if(HandleInputHeader() == -1)
        {
            return -2;
        }

        // Its possible on some error situations that this happens
        // for example on closing when epoll receives more chunked data and stuff
        // hope this is not hack ,as proper m_pRecvWorldPacket is asserted around
        if(!m_pRecvWorldPacket)
        {
            LOG(ERROR)<<"Forcing close on input m_pRecvWorldPacket = NULL";
            return -2;
        }

        // We have full read header, now check the data payload
        if(m_recvBuffer.space() > 0)
        {
            // need more data in the payload
            const size_t toDataSize = (message_block.length() > m_recvBuffer.space() ? m_recvBuffer.space() : message_block.space());
            m_recvBuffer.copy(message_block.rd_ptr(), toDataSize);
            message_block.rd_ptr(toDataSize);

            if(m_recvBuffer.space() > 0)
            {
                // Couldn't receive the whole data this time.
                Jovi_ASSERT(message_block.length() == 0);
                return -1;
            }
        }

        // just received fresh new payload
        if(HandleInputPayload() == -1)
        {
            return -1;
        }
    }

    memset(m_buffer, 0, SOCKET_READ_BUFFER_SIZE);

    m_socket->async_read_some(boost::asio::buffer(m_buffer, SOCKET_READ_BUFFER_SIZE), boost::bind(&WorldSocket::HandleInput, this));

    return size == SOCKET_READ_BUFFER_SIZE ? 1 : 2;
}

int WorldSocket::HandleInputHeader()
{
    Jovi_ASSERT(m_pRecvWorldPacket == NULL);

    Jovi_ASSERT(m_header.length() == sizeof(PacketHeader));

    PacketHeader& header = (*(PacketHeader*)m_header.rd_ptr());

    // ? endian convert

    // ? 
    if(header.size < 4 || header.size > 10240 || header.cmd > 10240)
    {
        // client sent malformed packet size = %d , cmd = %d
        LOG(ERROR)<<"client sent malformed packet size = "<<header.size<<" , cmd = "<<header.cmd;
        return -1;
    }

    header.size -= 4;

    m_pRecvWorldPacket = new WorldPacket(OpCodes(header.cmd), header.size);

    if(header.size > 0)
    {
        m_pRecvWorldPacket->resize(header.size);
        m_recvBuffer.base((char*)m_pRecvWorldPacket->contents(), m_pRecvWorldPacket->size());
    }
    else
    {
        Jovi_ASSERT(m_recvBuffer.space() == 0);
    }

    return 0;
}

int WorldSocket::HandleInputPayload()
{
    Jovi_ASSERT(m_recvBuffer.space() == 0);
    Jovi_ASSERT(m_header.space() == 0);
    Jovi_ASSERT(m_pRecvWorldPacket != NULL);

    const int ret = ProcessIncoming(m_pRecvWorldPacket);

    m_recvBuffer.base(NULL, 0);
    m_recvBuffer.reset();
    m_pRecvWorldPacket = NULL;

    m_header.reset();

    return ret;
}

int WorldSocket::HandleOutput()
{
    // unlock in callback func
    m_OutBufferLock.lock();

    if(m_isClose)
    {
        return -1;
    }

    const size_t sendSize = m_outBuffer->length();

    if(sendSize == 0)
    {
        m_OutBufferLock.unlock();
        return 0;
    }

    // We are using boost::asio::async_write(), 
    // rather than ip::tcp::socket::async_write_some(), 
    // to ensure that the entire block of data is sent.
    boost::asio::async_write(m_socket, boost::asio::buffer(m_outBuffer->rd_ptr(), sendSize), boost::bind(&WorldSocket::HandleAsyncWriteComplete, this));

    return 0;
}

void WorldSocket::HandleAsyncWriteComplete()
{
    // lock in HandleOutput
    m_outBuffer->reset();

    m_OutBufferLock.unlock();

    if(iFlushPacketQueue())
    {
        HandleOutput();
    }
}

bool WorldSocket::iFlushPacketQueue()
{
    if(m_PacketQueue.empty())
    {
        return false;
    }

    WorldPacket* pkt;
    bool haveone = false;

    while(!m_PacketQueue.empty())
    {
        pkt = *(m_PacketQueue.begin());
        m_PacketQueue.pop_front();

        if(iSendPacket(*pkt) == -1)
        {
            m_PacketQueue.push_front(pkt);
            break;
        }
        else
        {
            haveone = true;
            delete pkt;
        }
    }

    return haveone;
}

int WorldSocket::iSendPacket(const WorldPacket& pkt)
{
    if(m_outBuffer->space() < pkt.size() + sizeof(PacketHeader))
    {
        return -1;
    }

    PacketHeader header;

    header.cmd = pkt.getOpcode();

    // ? endian
    header.size = (uint16) pkt.size() + 4;

    // ? endian

    // crypt

    if(m_outBuffer->copy((char*)&header, sizeof(header)) == -1)
    {
        /*LOG(ERROR)<<"packet header copy failed, cmd: "<<header.cmd
        <<", header size: "<<sizeof(header)
        <<", m_outBuffer space: "<<m_outBuffer.space();*/
        Jovi_ASSERT(false);
    }

    if(!pkt.empty())
    {
        if(m_outBuffer->copy((char*)pkt.contents(), pkt.size()) == -1)
        {
            /*LOG(ERROR)<<"packet copy failed, cmd: "<<pkt.getOpcode()
            <<", packet size: "<<pkt.size()
            <<", m_outBuffer space: "<<m_outBuffer.space();*/
            Jovi_ASSERT(false);
        }
    }

    return 0;
}

int WorldSocket::sendPacket(const WorldPacket& packet)
{
    boost::mutex::scoped_lock guard(m_OutBufferLock);

    if(m_isClose)
    {
        return -1;
    }

    // ? message into a single files, TODO
    
    if(iSendPacket(packet) == -1)
    {
        WorldPacket* pkt = new WorldPacket(packet);
        m_PacketQueue.push_back(pkt);
    }

    return 0;
}