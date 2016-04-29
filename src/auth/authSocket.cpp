/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/27
*/

#include "authSocket.h"
#include "authCode.h"

struct AuthHandler
{
    AuthCmd     cmd;
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
    {CMD_AUTH_NULL,                 STATUS_CONNECTED,           &AuthSocket::HandleNull}
};

#define AUTH_TOTAL_REGISTED_COMMANDS    sizeof(table)/sizeof(AuthHandler)

///////////////////////////////////////////////////////////////////////////////

AuthSocket::AuthSocket()
    : m_socket(nullptr)
    , m_inputBuffer(m_buffer, SOCKET_READ_BUFFER_SIZE)
    , m_outBuffer(SOCKET_WRITE_BUFFER_SIZE)
    , m_authed(false)
{

}

AuthSocket::~AuthSocket()
{

}

void AuthSocket::OnAccept()
{
    LOG(INFO)<<"accepting connection from "<<getRemoteAddress();

    m_remoteAddress = m_socket->remote_endpoint().address().to_string();

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
        // ? handle error
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
    uint8 cmd = 0;
    while(1)
    {
        if(!recv_soft((char*)&cmd, 1))
        {
            break;
        }

        size_t i = 0;
        for(i = 0; i < AUTH_TOTAL_REGISTED_COMMANDS; ++i)
        {
            if((uint8)table[i].cmd == cmd && 
                (table[i].status == STATUS_CONNECTED ||
                (m_authed && table[i].status == STATUS_AUTHED)))
            {
                LOG(INFO)<<"[Auth] got data for cmd "<<cmd<<SEPARATOR_SPACE
                    <<"length "<<recv_len();

                if(!(*this.*table[i].handler)())
                {
                    LOG(ERROR)<<"Command handler failed for cmd "<<cmd<<SEPARATOR_SPACE
                        <<"length "<<recv_len();
                }
                break;
            }
        }

        /// report unknown commands in log
        if(i == AUTH_TOTAL_REGISTED_COMMANDS)
        {
            LOG(ERROR)<<"[Auth] get unknown packet "<<cmd;
            break;
        }
    }


    m_inputBuffer.crunch();
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
        return true;
    }

    m_outBufferLock.lock();

    if(m_outBuffer.copy(buf, len) == -1)
    {
        m_outBufferLock.unlock();
        return false;
    }

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
}

bool AuthSocket::HandleNull()
{
    return true;
}