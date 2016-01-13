/*
    a simple web server 
    my IP: 192.168.195.131
    input http://192.168.100.100 in chrome will display Hello,world!
*/

#include <boost/asio.hpp>
#include <string>

boost::asio::io_service io_service;
boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 80);
boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
boost::asio::ip::tcp::socket sock(io_service);

std::string data = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";

void HandlerWrite(const boost::system::error_code &ec, std::size_t bytes_transferred)
{

}

void HandlerAccept(const boost::system::error_code &ec)
{
    if(!ec)
    {
        boost::asio::async_write(sock, boost::asio::buffer(data), HandlerWrite);
    }
}

int main()
{
    acceptor.listen();
    acceptor.async_accept(sock, HandlerAccept);

    io_service.run();

    return 0;
}