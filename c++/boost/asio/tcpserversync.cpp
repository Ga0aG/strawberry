#include <chrono>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>

using namespace boost::asio;

// g++  boost/tcpclientsync.cpp -o build/boost/tcpclientsync  -lpthread

int main() {
    try {
        io_service service;
        ip::tcp::endpoint endpoint(ip::tcp::v4(), 12345);
        ip::tcp::acceptor acceptor(service, endpoint);

        std::cout << "Server started. Waiting for clients..." << std::endl;

        while (true) {
            ip::tcp::socket socket(service);
            // This function is used to accept a new connection from a peer into the given socket. The function call will block until a new connection has been accepted successfully or an error occurs.
            acceptor.accept(socket);

            std::cout << "Client connected: " << socket.remote_endpoint() << std::endl;

            // Handle communication with the client
            std::string message = "Hello from server!";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            boost::system::error_code ignored_error;
            write(socket, buffer(message), ignored_error);

            socket.close();
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}