/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/27
*/

#ifndef _AUTH_SOCKET_H_
#define _AUTH_SOCKET_H_

#include "Common.h"
#include "buffer.h"
#include <boost/thread/mutex.hpp>

class AuthSocket
{
public:
    
    AuthSocket();
    
    ~AuthSocket();

public:

    typedef boost::mutex                        LockType;


public:

    // check if socket is closed£¬´ý¶¨
    bool close() const;

    // set close flag£¬ ´ý¶¨
    void close(bool flag);

    // 
    void closeSocket();

    const std::string& getRemoteAddress() const
    {
        return m_remoteAddress;
    }

    // get bsocket
    BSocket* bsocket () const
    {
        return m_socket;
    }

    // set bsocket
    void bsocket(BSocket* sock)
    {
        m_socket = sock;
    }

public:

    void OnAccept();

    void OnRead(const boost::system::error_code &ec, size_t bytes_transferred);

    void OnAsyncWirte(const boost::system::error_code &ec, size_t bytes_transferred);

    void asyncRead();

public:

    size_t recv_len() const;

    bool recv_soft(char* buf, size_t len);

    bool recv(char* buf, size_t len);

    void recv_skip(size_t len);

    bool send(const char* buf, size_t len);

// handle auth code
public:

    bool HandleNull();

private:

    std::string     m_remoteAddress;
    BSocket*        m_socket;
    char            m_buffer[SOCKET_READ_BUFFER_SIZE];
    Buffer          m_inputBuffer;
    LockType        m_outBufferLock;
    Buffer          m_outBuffer;
    bool            m_authed;
};

#endif//_AUTH_SOCKET_H_