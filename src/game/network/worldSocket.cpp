/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/18
*/

#include "worldSocket.h"
#include "worldSocketMgr.h"
#include "worldPacket.h"
#include <boost/bind.hpp>


WorldSocket::WorldSocket()
    : m_lastPingTime(0)
    , m_overSpeedPings(0)
    , m_Session(NULL)
    , m_pRecvWorldPacket(NULL)
    , m_recvBuffer()
    , m_header(sizeof(PacketHeader))
    , m_outBuffer(NULL)
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

    try
    {
        m_address = m_socket->remote_endpoint().address().to_string();
    }
    catch(boost::system::system_error& ec)
    {
        StackTrace stack;
        LOG(ERROR)<<ec.what();
        LOG(ERROR)<<stack.c_str();
        return -1;
    }

    WorldPacket packet(MSG_EVIL_SOCKET_STARTUP, 4);
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

void WorldSocket::closeSocket()
{
    {
        boost::mutex::scoped_lock guard(m_OutBufferLock);

        if(m_isClose)
        {
            return;
        }

        m_isClose = true;
		        
		boost::system::error_code ec;
        m_socket->close(ec);
		if (ec)
		{
			LOG(ERROR) << boost::system::system_error(ec).what();
		}
    }

    {
        boost::mutex::scoped_lock guard(m_SessionLock);

        m_Session = NULL;
    }
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

#ifdef DEBUG_INFO_CONCURRENCE_TEST
int WorldSocket::HandleInputTest(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec)
    {
        LOG(ERROR)<<boost::system::system_error(ec).what();
        this->close(true);
        return -1;
    }

    // ? add other arguments later
    LOG(INFO)<<"input data size: "<<bytes_transferred;
    LOG(INFO)<<"input data: "<<m_buffer;

    memset(&m_buffer, 0, SOCKET_READ_BUFFER_SIZE);

    WorldPacket packet(MSG_EVIL_SOCKET_STARTUP, 4);
    packet << "5678";

#ifdef DEBUG_INFO_SOCKET_WRITE
    char* buffer = new char[m_outBuffer->length() + 1];
    memset(buffer, 0, m_outBuffer->length());
    memcpy(buffer, m_outBuffer->rd_ptr(), m_outBuffer->length());
    buffer[m_outBuffer->length()] = '\0';
    LOG(ERROR)<<"m_outBuffer: "<<buffer<<", size: "<<m_outBuffer->length()<<" in HandleInputTest";
    SafeDeleteArray(buffer);
#endif

#ifdef DEBUG_INFO_SOCKET_WRITE
    LOG(ERROR)<<"sendPacket in HandleInputTest";
#endif
    if(sendPacket(packet) == -1)
    {
        LOG(ERROR)<<"sendPacket failed";
    }
#ifdef DEBUG_INFO_SOCKET_WRITE
    LOG(ERROR)<<"sendPacket in HandleInputTest success";
#endif

    m_socket->async_read_some(boost::asio::buffer(m_buffer, SOCKET_READ_BUFFER_SIZE), 
        boost::bind(&WorldSocket::HandleInputTest, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

    return 0;
}
#endif

void WorldSocket::HandleInput(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec)
    {
        LOG(ERROR)<<boost::system::system_error(ec).what();

		if (this->bsocket() == sWorldSocketMgr.getAuthBSocket())
		{
			this->closeSocket();
			sWorldSocketMgr.registToAuth();
		}

        return;
    }

    LOG(INFO)<<"input data size: "<<bytes_transferred;
	if (bytes_transferred == 0)
	{
		return;
	}

    if(m_isClose)
    {
        return;
    }

#ifdef DEBUG_INFO_WRITE_AND_READ
    PacketHeader header;
    memcpy(&header, m_buffer, bytes_transferred);
    ELOG(ERROR) << "cmd : " << header.cmd << SEPARATOR_COMMA << "size : " << header.size;

    EndianConvert(header);

	char t;
	memcpy(&t, m_buffer, 1);
    ELOG(ERROR) << "cmd : " << header.cmd << SEPARATOR_COMMA << "size : " << header.size;
	ELOG(ERROR) << "strlen buf size : " << strlen(m_buffer) << " sizeof "<< sizeof(m_buffer) << " t " <<(int)t;
#endif

    switch(HandleInputMissingData(bytes_transferred))
    {
    case -1:
        {
            if(errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // ? why do this
				Update();
				return;
            }

            LOG(INFO)<<"WorldSocket::handle_input: Peer error closing connection errno = "<<errno;
        
            errno = ECONNRESET;
        }
		break;
    case 0:
        {
            // LOG(ERROR)<<"WorldSocket::handle_input: Peer has closed connection";
			LOG(ERROR) << "recv data size is zero";

            errno = ECONNRESET;
        }
		break;
    case 1:
        break;
    default:
        {
			Update();
		}
		break;
    }

    return;
}

int WorldSocket::HandleInputMissingData(size_t len)
{
    if(0 == len)
    {
        return 0;
    }

    Buffer message_block(m_buffer, len);

    message_block.wr_ptr(len);
	
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
            Evil_ASSERT(message_block.length() == 0);
            errno = EWOULDBLOCK;
            return -1;// EWOULDBLOCK
        }

        // received a nice new header
        if(HandleInputHeader() == -1)
        {
            Evil_ASSERT(errno != EWOULDBLOCK && errno != EAGAIN)
            return -1;
        }

        // Its possible on some error situations that this happens
        // for example on closing when epoll receives more chunked data and stuff
        // hope this is not hack ,as proper m_pRecvWorldPacket is asserted around
        if(m_pRecvWorldPacket == nullptr)
        {
            LOG(ERROR)<<"Forcing close on input m_pRecvWorldPacket = NULL";
            errno = EAGAIN;
            return -1;
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
                Evil_ASSERT(message_block.length() == 0);
                errno = EWOULDBLOCK;
                return -1;
            }
        }

        // just received fresh new payload
        if(HandleInputPayload() == -1)
        {
            Evil_ASSERT((errno != EWOULDBLOCK) && (errno != EAGAIN));
            return -1;
        }
    }

    memset(m_buffer, 0, SOCKET_READ_BUFFER_SIZE);

    m_socket->async_read_some(boost::asio::buffer(m_buffer, SOCKET_READ_BUFFER_SIZE), 
        boost::bind(&WorldSocket::HandleInput, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

    return len == SOCKET_READ_BUFFER_SIZE ? 1 : 2;
}

int WorldSocket::HandleInputHeader()
{
    Evil_ASSERT(m_pRecvWorldPacket == NULL);

    Evil_ASSERT(m_header.length() == sizeof(PacketHeader));

    PacketHeader& header = (*(PacketHeader*)m_header.rd_ptr());

#ifdef DEBUG_INFO_WRITE_AND_READ
    ELOG(ERROR) << "cmd : " << header.cmd << SEPARATOR_COMMA << "size : " << header.size;
#endif

    // consider the element(size/cmd) of the PacketHeader could be converter(endian) respectively
    EndianConvert(header);

#ifdef DEBUG_INFO_WRITE_AND_READ
    ELOG(ERROR) << "cmd : " << header.cmd << SEPARATOR_COMMA << "size : " << header.size;
#endif

    if(header.size < sizeof(PacketHeader) || header.size > MSG_LENGTH_MAX || header.cmd >= MSG_MAX)
    {
        // client sent malformed packet size = %d , cmd = %d
        LOG(ERROR)<<"client sent malformed packet size = "<<header.size<<SEPARATOR_COMMA<<"cmd = "<<header.cmd;
        return -1;
    }

    header.size -= sizeof(PacketHeader);

    m_pRecvWorldPacket = new WorldPacket(OpCodes(header.cmd), header.size);

    if(header.size > 0)
    {
        m_pRecvWorldPacket->resize(header.size);
        m_recvBuffer.base((char*)m_pRecvWorldPacket->contents(), m_pRecvWorldPacket->size());
    }
    else
    {
        Evil_ASSERT(m_recvBuffer.space() == 0);
    }

    return 0;
}

int WorldSocket::HandleInputPayload()
{
    Evil_ASSERT(m_recvBuffer.space() == 0);
    Evil_ASSERT(m_header.space() == 0);
    Evil_ASSERT(m_pRecvWorldPacket != nullptr);

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
        m_OutBufferLock.unlock();
        return -1;
    }

    const size_t sendSize = m_outBuffer->length();

    if(sendSize == 0)
    {
        m_OutBufferLock.unlock();
        return 0;
    }

    /*
        We are using boost::asio::async_write(), 
        rather than ip::tcp::socket::async_write_some(), 
        to ensure that the entire block of data is sent.

        be careful managing the buf objectLefttime
        linux: data in buffer has copy when async_write return
        windows: data maybe not written when async_write return
    */
    boost::asio::async_write(*m_socket, boost::asio::buffer(m_outBuffer->rd_ptr(), sendSize), 
        boost::bind(&WorldSocket::HandleAsyncWriteComplete, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

    return 0;
}

void WorldSocket::HandleAsyncWriteComplete(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec)
    {
        LOG(ERROR)<<boost::system::system_error(ec).what();
        m_OutBufferLock.unlock();
        this->closeSocket();
        return;
    }

    LOG(INFO)<<"output data size: "<<bytes_transferred;

    // locked in HandleOutput
    if(m_outBuffer->length() == bytes_transferred)
    {
        m_outBuffer->reset();

        bool isFlush = iFlushPacketQueue();

        // lock in HandleOutput, if do not unlock, myybe deadlock
        m_OutBufferLock.unlock();

        if(isFlush)
        {
            HandleOutput();
        }
    }
    else
    {
        m_outBuffer->rd_ptr(bytes_transferred);
        m_outBuffer->crunch();
        m_OutBufferLock.unlock();
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
    header.size = static_cast<uint16>(pkt.size() + sizeof(header));

    EndianConvert(header);

    // crypt

    if(m_outBuffer->space() < sizeof(header))
    {
        Evil_ASSERT(false);
    }
    m_outBuffer->copy((char*)&header, sizeof(header));


    if(!pkt.empty())
    {
        if(m_outBuffer->space() < pkt.size())
        {
            Evil_ASSERT(false);
        }
        m_outBuffer->copy((char*)pkt.contents(), pkt.size());
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

int WorldSocket::ProcessIncoming(WorldPacket* pPkt)
{
    Evil_ASSERT(pPkt);

    int ret = 0;
    uint16 opcode = pPkt->getOpcode();
    switch(opcode)
    {
    case MSG_AUTH_EVIL_REGIST_ACK:
        {
            ret = HandleAuthRegisterAck();
        }
        break;
    default:
        {

        }
        break;
    }

    return ret;
}

int WorldSocket::HandleConnect()
{
    // prevent double call to this func
    if(m_outBuffer)
    {
        return -1;
    }

    close(false);

    if (m_socket == nullptr)
    {
        ELOG(ERROR) << "connector socket point is nullptr";
        return -1;
    }

    //  new buffer
    m_outBuffer = new Buffer(m_outBufferSize);
    
    m_address = m_socket->remote_endpoint().address().to_string();

    WorldPacket packet(MSG_AUTH_EVIL_REGIST, 0);

    if(sendPacket(packet) == -1)
    {
        return -1;
    }

    m_socket->async_read_some(boost::asio::buffer(m_buffer, SOCKET_READ_BUFFER_SIZE),
        boost::bind(&WorldSocket::HandleInput, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

    return 0;
}

int WorldSocket::HandleAuthRegisterAck()
{
    LOG(INFO) << "[Evil] regist auth server successfully";

    return 0;
}