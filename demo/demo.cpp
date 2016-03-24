#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <list>

typedef boost::asio::io_service                 Proactor;
typedef boost::asio::ip::tcp::acceptor          Acceptor;

void OnWriteComplete(const boost::system::error_code &ec, size_t bytes_transferred);

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

    std::list<boost::asio::ip::tcp::socket*>& getSocketList()
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

        boost::asio::deadline_timer timer(*m_Proactor, boost::posix_time::microsec(10000));
        timer.async_wait(boost::bind(&ProactorRunnable::threadLoop, this, boost::ref(timer)));

        m_Proactor->run();
    }

    void threadLoop(boost::asio::deadline_timer &timer)
    {
        if(m_Proactor->stopped())
        {
            std::cout<<"m_Proactor is stopped"<<std::endl;
            return;
        }

        if(!socketList.empty())
        {
            char buf[10] = {0};
            memcpy(buf, "abcd", 4);
            for(std::list<boost::asio::ip::tcp::socket*>::iterator iter = socketList.begin(); iter != socketList.end(); ++iter)
            {
                boost::asio::async_write(*iter, boost::asio::buffer(buf, 4),
                    boost::bind(&OnWriteComplete, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            }
        }

        timer.expires_from_now(boost::posix_time::microsec(10000));
        timer.async_wait(boost::bind(&ProactorRunnable::threadLoop, this, boost::ref(timer)));
    }
private:

    Proactor*                   m_Proactor;     // wrap boost io_service
    boost::thread*              m_pThread;
    std::list<boost::asio::ip::tcp::socket*>    socketList;
};

void OnWriteComplete(const boost::system::error_code &ec, size_t bytes_transferred)
{
    std::cout<<"OnWriteComplete was invoked"<<std::endl;
}

void OnSocketOpen(const boost::system::error_code &ec, boost::asio::ip::tcp::socket *bsocket, std::list<boost::asio::ip::tcp::socket*>& socketList)
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
    bsocket->set_option(option);

    socketList.push_back(bsocket);
}

int main()
{
    ProactorRunnable* pNetThread = new ProactorRunnable[2];

    pNetThread[0].start();
    pNetThread[1].start();

    Acceptor* acceptor = new Acceptor(*(pNetThread[0].proactor()));
    
    boost::asio::ip::tcp::endpoint localEndpoint(boost::asio::ip::tcp::v4(), 10302);

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

    boost::asio::ip::tcp::socket *bsocket = new boost::asio::ip::tcp::socket(*(pNetThread[1].proactor()));

    acceptor->async_accept(*bsocket, 
        boost::bind(&OnSocketOpen, boost::asio::placeholders::error, bsocket, pNetThread[0].getSocketList()));

    pNetThread[0].wait();
    pNetThread[1].wait();

    return 0;
}