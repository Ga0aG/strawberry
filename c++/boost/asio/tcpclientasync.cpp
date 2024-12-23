#include <boost/asio.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    using boost::asio::ip::tcp;
    boost::asio::io_context io_context;

    // we need a socket and a resolver
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);

    // now we can use connect(..)
    // resolve 是用于将主机名（如域名）解析为 IP 地址的过程。它通常涉及到 DNS 查询。这个过程是异步的，允许程序在等待解析完成时继续执行其他操作。
    boost::asio::connect(socket, resolver.resolve("127.0.0.1", "12345"));

    // and use write(..) to send some data which is here just a string
    std::string data{"some client data ..."};
    auto result = boost::asio::write(socket, boost::asio::buffer(data));

    // the result represents the size of the sent data
    std::cout << "data sent: " << data.length() << '/' << result << std::endl;

    // and close the connection now
    boost::system::error_code ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket.close();

    return 0;
}
