#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE] = {0};

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    std::string hello = "Hello from client";
    send(sock, hello.c_str(), hello.size(), 0);
    std::cout << "Hello message sent" << std::endl;

    int valread = read(sock, buffer, BUF_SIZE);
    std::cout << "Received from server: " << buffer << std::endl;

    hello = "Hello from client again";
    send(sock, hello.c_str(), hello.size(), 0);
    std::cout << "Hello message sent" << std::endl;
    valread = read(sock, buffer, BUF_SIZE);
    std::cout << "Received from server: " << buffer << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
    close(sock);
    std::cout << "Exiting" << buffer << std::endl;
    return 0;
}
