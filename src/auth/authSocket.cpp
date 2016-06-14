/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/27
*/

#include "authSocket.h"
#include "code.h"
#include "byteConverter.h"
#include "authSocketMgr.h"

struct AuthHandler
{
    OpCodes     cmd;
    uint        status;
    bool(AuthSocket::*handler)(void);
};

enum SocketStatus
{
    STATUS_CONNECTED = 0,
    STATUS_AUTHED,
};

// register authCode
const AuthHandler table[] = 
{
    {MSG_NULL_ACTION,                   STATUS_CONNECTED,           &AuthSocket::HandleNull         },
    {MSG_AUTH_EVIL_REGIST,              STATUS_CONNECTED,           &AuthSocket::HandleEvilRegist   },
    {MSG_AUTH_EVIL_UNREGIST,            STATUS_CONNECTED,           &AuthSocket::HandleEvilUnregist },
};

#define AUTH_TOTAL_REGISTED_COMMANDS    sizeof(table)/sizeof(AuthHandler)

///////////////////////////////////////////////////////////////////////////////

AuthSocket::AuthSocket()
    : m_port(0)
    , m_socket(nullptr)
    , m_inputBuffer(m_buffer, SOCKET_READ_BUFFER_SIZE)
    , m_outBuffer(SOCKET_WRITE_BUFFER_SIZE)
    , m_authed(false)
	, m_close(false)
    , m_isEvil(false)
{

}

AuthSocket::~AuthSocket()
{
	
}

void AuthSocket::OnAccept()
{
    m_remoteAddress = m_socket->remote_endpoint().address().to_string();
    m_port = m_socket->remote_endpoint().port();

    LOG(INFO)<<"accepting connection from "<<getRemoteAddress();

    // TCP_NODELAY
    boost::asio::ip::tcp::no_delay option(true);
    m_socket->set_option(option);

    asyncRead();
}

void AuthSocket::OnRead(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec)
    {
        LOG(ERROR)<<boost::system::system_error(ec).what();
        
        if(isEvil())
        {
            sAuthSockMgr.eraseServer(m_remoteAddress, m_port);
        }
        this->closeSocket();

        return;
    }

    LOG(INFO)<<"input data size : "<<bytes_transferred<<SEPARATOR_COMMA
        <<"inputbuffer space : "<<m_inputBuffer.space();

    if(0 == bytes_transferred)
    {
        return;
    }

    // ? Is m_inputBuffer init ??
    this->m_inputBuffer.wr_ptr(bytes_transferred);

    /// handle message
    PacketHeader header;
    while(1)
    {
        if(!recv_soft((char*)&header, sizeof(PacketHeader)))
        {
            break;
        }
        
        EndianConvert(header);

        size_t i = 0;
        for(i = 0; i < AUTH_TOTAL_REGISTED_COMMANDS; ++i)
        {
            if((uint8)table[i].cmd == header.cmd && 
                (table[i].status == STATUS_CONNECTED ||
                (m_authed && table[i].status == STATUS_AUTHED)))
            {
                LOG(INFO)<<"[Auth] got data for cmd "<<header.cmd<<SEPARATOR_SPACE
                    <<"length "<<recv_len();

                if(!(*this.*table[i].handler)())
                {
                    LOG(ERROR)<<"Command handler failed for cmd "<<header.cmd<<SEPARATOR_SPACE
                        <<"length "<<recv_len();
                }
                break;
            }
        }

        /// report unknown commands in log
        if(i == AUTH_TOTAL_REGISTED_COMMANDS)
        {
            LOG(ERROR)<<"[Auth] get unknown packet "<<header.cmd;
            memset(m_inputBuffer.base(), 0, SOCKET_READ_BUFFER_SIZE);
            m_inputBuffer.reset();
            break;
        }
    }

    m_inputBuffer.crunch();

	asyncRead();
}

void AuthSocket::asyncRead()
{
    m_socket->async_read_some(boost::asio::buffer(m_inputBuffer.wr_ptr(), m_inputBuffer.space()),
        boost::bind(&AuthSocket::OnRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

bool AuthSocket::recv_soft(char* buf, size_t len)
{
    if(buf == nullptr)
    {
        LOG(ERROR)<<"input buf is NULL";
        return false;
    }

#ifdef DEBUG_INFO_CONNECT
    ELOG(ERROR) << "m_inputBuffer length : " << m_inputBuffer.length() << SEPARATOR_COMMA << "input len : " << len;
#endif

    if(this->m_inputBuffer.length() < len)
    {
        return false;
    }

    memcpy(buf, this->m_inputBuffer.rd_ptr(), len);

    return true;
}

size_t AuthSocket::recv_len() const
{
    return m_inputBuffer.length();
}

void AuthSocket::recv_skip(size_t len)
{
    m_inputBuffer.rd_ptr(len);
}

bool AuthSocket::recv(char* buf, size_t len)
{
    bool ret = recv_soft(buf, len);

    if(ret)
    {
        recv_skip(len);
    }

    return ret;
}

bool AuthSocket::send(const char* buf, size_t len)
{
    if(buf == nullptr || len == 0)
    {
        return false;
    }

    m_outBufferLock.lock();

    if(m_outBuffer.space() < len)
    {
        m_outBufferLock.unlock();
        return false;
    }
    m_outBuffer.copy(buf, len);

    boost::asio::async_write(*m_socket, boost::asio::buffer(m_outBuffer.rd_ptr(), m_outBuffer.length()), 
        boost::bind(&AuthSocket::OnAsyncWirte, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    
    return true;
}

void AuthSocket::OnAsyncWirte(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec)
    {
        LOG(ERROR)<<boost::system::system_error(ec).what();
        m_outBufferLock.unlock();

        this->closeSocket();
        return;
    }

    LOG(INFO)<<"output data size: "<<bytes_transferred;

    if(m_outBuffer.length() == bytes_transferred)
    {
        m_outBuffer.reset();
    }
    else
    {
        m_outBuffer.rd_ptr(bytes_transferred);
        m_outBuffer.crunch();
    }

    m_outBufferLock.unlock();

    return;
}

bool AuthSocket::HandleNull()
{
    return true;
}

bool AuthSocket::HandleEvilRegist()
{
    recv_skip(sizeof(PacketHeader));

    PacketHeader header;
    
    // to evil
    header.size = sizeof(header);
    header.cmd = MSG_AUTH_EVIL_REGIST_ACK;

    EndianConvert(header);

    Buffer outbuf(sizeof(header));
    outbuf.copy((char*)&header, sizeof(header));

    send(outbuf.rd_ptr(), outbuf.length());

    sAuthSockMgr.addServer(m_remoteAddress, m_port);
    m_isEvil = true;

    return true;
}

bool AuthSocket::HandleEvilUnregist()
{
	sAuthSockMgr.eraseServer(m_remoteAddress, m_port);

    return true;
}