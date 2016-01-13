/*
    基于Boost.Asio来提高程序的可扩展性推荐的方法是:
        采用单个I/O service多次调用run()方法

    也有另外的方法可以选择:
        可以创建多个I/O service 而不是将所有的线程都绑定到一个I/O service上
        每个 I/O service 对应于一个线程 
        如果 I/O service 的个数和计算机的核数相匹配 异步操作将会在各自对应的核上运行
*/

#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

void handler1(const boost::system::error_code &ec)
{
    std::cout<<"5 s"<<std::endl;
}

void handler2(const boost::system::error_code &ec)
{
    std::cout<<"5 s"<<std::endl;
}

boost::asio::io_service io_service;

void run()
{
    io_service.run();
}

int main()
{
    boost::asio::deadline_timer timer1(io_service, boost::posix_time::seconds(5));
    timer1.async_wait(handler1);

    boost::asio::deadline_timer timer2(io_service, boost::posix_time::seconds(5));
    timer2.async_wait(handler2);

    boost::thread thread1(run);
    boost::thread thread2(run);

    thread1.join();
    thread2.join();

    return 0;
}