#include <iostream>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include "../include/helper.hh"

#define MAX_EVENTS 10
#define PORT 8080
#define BUF_SIZE 1024

// Reference: chatgpt

int main()
{
  int server_fd, new_socket, epoll_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  struct epoll_event ev, events[MAX_EVENTS];

  // Create server socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  INFO_STREAM("Create server fd: " << epoll_fd);

  // Bind the socket
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // Listen on the socket
  if (listen(server_fd, 3) < 0)
  {
    perror("listen");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // Create epoll instance
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1)
  {
    perror("epoll_create1");
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  INFO_STREAM("Create epoll fd: " << epoll_fd);

  // Add server socket to epoll
  ev.events = EPOLLIN;
  ev.data.fd = server_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
  {
    perror("epoll_ctl: server_fd");
    close(server_fd);
    close(epoll_fd);
    exit(EXIT_FAILURE);
  }

  while (true)
  {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1)
    {
      perror("epoll_wait");
      close(server_fd);
      close(epoll_fd);
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nfds; ++i)
    {
      if (events[i].data.fd == server_fd)
      {
        // Accept new connection, create new socket
        // 每次调用 accept() 后，都会生成一个新的 socket，用于与该客户端进行通信。这是因为每个 socket 实例负责管理特定客户端的数据传输，通过这种方式可以实现多客户端并发连接。
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket == -1)
        {
          perror("accept");
        }
        else
        {
          ev.events = EPOLLIN;
          ev.data.fd = new_socket;
          INFO_STREAM("Create new socket: " << new_socket << ", server_fd: " << server_fd);
          if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev) == -1)
          {
            perror("epoll_ctl: new_socket");
            close(new_socket);
          }
        }
      }
      else
      {
        // Handle client data
        char buffer[BUF_SIZE] = {0};
        int client_fd = events[i].data.fd;
        int bytes_read = read(client_fd, buffer, BUF_SIZE);
        if (bytes_read <= 0)
        {
          if (bytes_read == 0)
          {
            // Connection closed
            std::cout << "Client disconnected" << std::endl;
          }
          else
          {
            perror("read");
          }
          close(client_fd);
        }
        else
        {
          INFO_STREAM("I'm " << client_fd); // < 10
          std::cout << "Received: " << buffer << std::endl;
          std::string hello = "Hello from client";
          send(client_fd, hello.c_str(), hello.size(), 0);
        }
      }
    }
  }

  close(server_fd);
  close(epoll_fd);
  return 0;
}