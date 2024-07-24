#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <csignal>
#include <deque>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <thread>
#include <unistd.h>

#define PORT 8080
#define BUF_SIZE 1024
#define MAX_EPOLL_EVENTS 16
bool running = true;
void signal_handler(int signal_num)
{
    std::cout << "The interrupt signal is (" << signal_num << "). " << std::endl;
    running = false;
};
int main()
{
    signal(SIGINT, signal_handler);
    // Create socket
    int socket_fd = 0;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        close(socket_fd);
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Edit retry times
    int syn_retries_old = -1;
    socklen_t syn_retries_len = sizeof(syn_retries_old);
    getsockopt(
        socket_fd,
        IPPROTO_TCP,
        TCP_SYNCNT,
        &syn_retries_old,
        &syn_retries_len);

    int syn_retries_timeout = 2;
    setsockopt(
        socket_fd,
        IPPROTO_TCP,
        TCP_SYNCNT,
        &syn_retries_timeout,
        sizeof(syn_retries_timeout));

    // Connect to server
    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    setsockopt(
        socket_fd,
        IPPROTO_TCP,
        TCP_SYNCNT,
        &syn_retries_old,
        sizeof(syn_retries_old));

    char buffer[BUF_SIZE] = {0};
    struct epoll_event event, events[MAX_EPOLL_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    int epfd = epoll_create1(0);
    epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, &event);

    std::deque<std::string> send_deque;
    std::string hello = "Hello from client";
    send_deque.push_back(hello);
    hello += "2";
    send_deque.push_back(hello);
    while (running)
    {
        int num_ready = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, 50); //milliseconds
        for (int i = 0; i < num_ready; ++i)
        {
            if (events[i].events & EPOLLIN && socket_fd == events[i].data.fd)
            {
                int n = recv(socket_fd, buffer, BUF_SIZE, 0);
                if (n < 0)
                {
                    std::cerr << "Failed to receive data" << std::endl;
                }
                else if (n == 0)
                {
                    std::cerr << "Server closes connection" << std::endl;
                }
                else
                {
                    // buffer[n] = 0;
                    // std::cout << "recv[";
                    // for (int i = 0; i < n; ++i)
                    // {
                    //     std::cout << std::setfill('0') << std::setw(2) << std::hex
                    //               << static_cast<int>(buffer[i]) << std::dec << ' ';
                    // }
                    // std::cout << "]" << std::endl;
                    std::cout << buffer << std::endl;
                }
            }
        }
        if (send_deque.size() > 0)
        {
            std::string &msg = send_deque.front();
            int s = send(socket_fd, msg.data(), msg.length(), 0);
            if (s < 0)
            {
                std::cerr << "Failed to send msg" << std::endl;
            }
            else if (s != static_cast<int>(msg.length()))
            {
                std::cerr << "Failed to send msg" << std::endl;
            }
            send_deque.pop_front();
        }
    }

    close(socket_fd);
    std::cout << "Bye" << std::endl;
    return 0;
}
