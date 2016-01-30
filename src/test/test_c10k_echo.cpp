#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

using namespace std;
using boost::asio::ip::tcp;

class io_service_pool
    : public boost::noncopyable
{
public:

    explicit io_service_pool(std::size_t pool_size)
        : next_io_service_(0)
    {           
        for (std::size_t i = 0; i < pool_size; ++ i)
        {
            io_service_sptr io_service(new boost::asio::io_service);
            work_sptr work(new boost::asio::io_service::work(*io_service));
            io_services_.push_back(io_service);
            work_.push_back(work);
        }
    }

    void start()
    {       
        for (std::size_t i = 0; i < io_services_.size(); ++ i)
        {
            boost::shared_ptr<boost::thread> thread(new boost::thread(
                boost::bind(&boost::asio::io_service::run, io_services_[i])));
            threads_.push_back(thread);
        }
    }

    void join()
    {
        for (std::size_t i = 0; i < threads_.size(); ++ i)
        {
            threads_[i]->join();
        }       
    }

    void stop()
    {       
        for (std::size_t i = 0; i < io_services_.size(); ++ i)
        {
            io_services_[i]->stop();
        }
    }

    boost::asio::io_service& get_io_service()
    {
        boost::mutex::scoped_lock lock(mtx);
        boost::asio::io_service& io_service = *io_services_[next_io_service_];
        ++ next_io_service_;
        if (next_io_service_ == io_services_.size())
        {
            next_io_service_ = 0;
        }
        return io_service;
    }

private:
    typedef boost::shared_ptr<boost::asio::io_service> io_service_sptr;
    typedef boost::shared_ptr<boost::asio::io_service::work> work_sptr;
    typedef boost::shared_ptr<boost::thread> thread_sptr;

    boost::mutex mtx;

    std::vector<io_service_sptr> io_services_;
    std::vector<work_sptr> work_;
    std::vector<thread_sptr> threads_;       
    std::size_t next_io_service_;
};

boost::mutex cout_mtx;
int packet_size = 0;
enum {MAX_PACKET_LEN = 4096};

class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
        , recv_times(0)
    {
    }

    virtual ~session()
    {
        boost::mutex::scoped_lock lock(cout_mtx);
        cout << recv_times << endl;
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    inline void start()
    {
        boost::asio::async_read(socket_, boost::asio::buffer(data_, packet_size),
            boost::bind(&session::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));           
    }

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (!error)
        {
            ++ recv_times;
            boost::asio::async_write(socket_, boost::asio::buffer(data_, bytes_transferred),
                boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            start();
        }
        else
        {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    char data_[MAX_PACKET_LEN];
    int recv_times;
};

class server
{
public:
    server(short port, int thread_cnt)
        : io_service_pool_(thread_cnt)
        , acceptor_(io_service_pool_.get_io_service(), tcp::endpoint(tcp::v4(), port))
    {
        session* new_session = new session(io_service_pool_.get_io_service());
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session, boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session, const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_pool_.get_io_service());
            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, new_session, boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

    void run()
    {
        io_service_pool_.start();
        io_service_pool_.join();
    }

private:

    io_service_pool io_service_pool_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 4)
        {
            cerr << "Usage: async_tcp_echo_server <port> <packet_size> <thread_cnt>" << endl;
            return 1;
        }
        short port = boost::lexical_cast<short>(argv[1]);
        packet_size = boost::lexical_cast<int>(argv[2]);
        int thread_cnt = boost::lexical_cast<int>(argv[3]);
        if (packet_size <= 0 || packet_size > MAX_PACKET_LEN)
        {
            cerr << "packet_size <= 0 || packet_size > MAX_PACKET_LEN" << endl;
            return 1;
        }
        if (thread_cnt <= 0)
        {
            cerr << "thread_cnt <= 0" << endl;
            return 1;           
        }       

        server s(port, thread_cnt);
        s.run();
    }
    catch (std::exception& e)
    {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}