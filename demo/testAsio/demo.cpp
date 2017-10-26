#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <list>

typedef boost::asio::io_service                 Proactor;
typedef boost::asio::ip::tcp::acceptor          Acceptor;

class Socket
{
public:
    Socket(boost::asio::io_service& proactor)
    {
        m_socket = new boost::asio::ip::tcp::socket(proactor);
    }

    void OnWriteComplete(const boost::system::error_code &ec, size_t bytes_transferred)
    {
        std::cout<<"OnWriteComplete was invoked"<<std::endl;

        char buf[10] = {0};
        memcpy(buf, "abcd", 4);
        boost::asio::async_write(*m_socket, boost::asio::buffer(buf, 4),
            boost::bind(&Socket::OnWriteComplete1, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void OnWriteComplete1(const boost::system::error_code &ec, size_t bytes_transferred)
    {
        std::cout<<"OnWriteComplete1 was invoked"<<std::endl;
    }

    boost::asio::ip::tcp::socket* getSocket() const
    {
        return m_socket;
    }
private:
    boost::asio::ip::tcp::socket * m_socket;
};

class ProactorRunnable
{
public:
    ProactorRunnable()
        : m_Proactor(0)
        , m_pThread(NULL)
    {
        m_Proactor = new Proactor;
    }

    ~ProactorRunnable()
    {
        stop();
        wait();

        delete m_pThread;
        delete m_Proactor;
    }

    bool start()
    {
        if(m_pThread)
        {
            return false;
        }

        boost::function0<void> threadTaskFun = boost::bind(&ProactorRunnable::threadTask, this);
        m_pThread = new boost::thread(threadTaskFun);

        return true;
    }

    void stop()
    {
        if(m_Proactor && !m_Proactor->stopped())
        {
            m_Proactor->stop();
        }
    }

    void wait()
    {
        if(m_pThread)
        {
            m_pThread->join();
        }
    }

    Proactor* proactor() const
    {
        return m_Proactor;
    }

    std::list<Socket*>& getSocketList()
    {
        return socketList;
    }
private:

    void threadTask()
    {
        if(!m_Proactor)
        {
            std::cout<<"m_Proactor is NULL"<<std::endl;
            return;
        }

        boost::asio::io_service::work work(*m_Proactor);

        boost::asio::deadline_timer timer(*m_Proactor, boost::posix_time::seconds(1));
        timer.async_wait(boost::bind(&ProactorRunnable::threadLoop, this, boost::ref(timer)));

        m_Proactor->run();
    }

    void threadLoop(boost::asio::deadline_timer &timer)
    {
        std::cout<<"timer, "<<"socketListSize: "<<socketList.size()<<std::endl;
        if(m_Proactor->stopped())
        {
            std::cout<<"m_Proactor is stopped"<<std::endl;
            return;
        }

        if(!socketList.empty())
        {
            char buf[10] = {0};
            memcpy(buf, "abcd", 4);
            for(std::list<Socket*>::iterator iter = socketList.begin(); iter != socketList.end(); ++iter)
            {
                std::cout<<"async_write"<<std::endl;
                boost::asio::async_write(*((*iter)->getSocket()), boost::asio::buffer(buf, 4),
                    boost::bind(&Socket::OnWriteComplete, *iter, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            }
        }

        timer.expires_from_now(boost::posix_time::seconds(1));
        timer.async_wait(boost::bind(&ProactorRunnable::threadLoop, this, boost::ref(timer)));
    }
private:

    Proactor*                   m_Proactor;     // wrap boost io_service
    boost::thread*              m_pThread;
    std::list<Socket*>    socketList;
};

void OnSocketOpen(const boost::system::error_code &ec, Socket* bsocket, std::list<Socket*>& socketList)
{
    if(!bsocket)
    {
        std::cout<<"bsocket is NULL"<<std::endl;
        return;
    }

    if(ec)
    {
        std::cout<<boost::system::system_error(ec).what()<<std::endl;
        return;
    }

    boost::asio::ip::tcp::no_delay option(true);
    bsocket->getSocket()->set_option(option);

    socketList.push_back(bsocket);
    std::cout<<"socket push back to socketlist"<<std::endl;
}

int main()
{
    ProactorRunnable* pNetThread = new ProactorRunnable[2];

    pNetThread[0].start();
    pNetThread[1].start();

    Acceptor* acceptor = new Acceptor(*(pNetThread[0].proactor()));
    
    boost::asio::ip::address addr;
    addr.from_string("192.168.195.131");
    boost::asio::ip::tcp::endpoint localEndpoint(addr, 10302);

    try
    {
        acceptor->open(localEndpoint.protocol());
        acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor->bind(localEndpoint);
        acceptor->listen();
    }
    catch(boost::system::system_error& ec)
    {
        std::cout<<ec.what()<<std::endl;
        return -1;
    }

    std::cout<<"listen: "<<addr.to_string()<<std::endl;

    Socket *bsocket = new Socket(*(pNetThread[1].proactor()));

    acceptor->async_accept(*(bsocket->getSocket()), 
        boost::bind(&OnSocketOpen, boost::asio::placeholders::error, bsocket, boost::ref(pNetThread[1].getSocketList())));

    pNetThread[0].wait();
    pNetThread[1].wait();

    return 0;
}
