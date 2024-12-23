#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;

int main() {
    try {
        // 是 Boost.Asio 中的核心组件之一，负责管理异步操作的执行。它提供了一个事件循环，允许你的程序在等待操作完成时继续执行其他任务。当有异步操作完成时，io_service 会通知相应的处理程序（如回调函数）。
        io_service service;
        ip::tcp::endpoint endpoint(ip::address::from_string("127.0.0.1"), 12345);
        ip::tcp::socket socket(service);
        socket.connect(endpoint);

        std::cout << "Connected to server!" << std::endl;

        // Receive message from server
        boost::asio::streambuf receive_buffer;
        boost::system::error_code error;
        read(socket, receive_buffer, transfer_all(), error);

        if (error && error != boost::asio::error::eof) {
            std::cerr << "Receive failed: " << error.message() << std::endl;
        } else {
            std::string message(boost::asio::buffers_begin(receive_buffer.data()),
                                boost::asio::buffers_end(receive_buffer.data()));
            std::cout << "Received message from server: " << message << std::endl;
        }

        socket.shutdown(ip::tcp::socket::shutdown_both);
        socket.close();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
