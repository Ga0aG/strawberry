#include <chrono>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Usage: linux> telnet 127.0.0.1 8080

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in address;
    fd_set read_fds;
    int max_fd, activity, i, valread;
    char buffer[BUFFER_SIZE] = {0};

    // 初始化客户端套接字数组
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // 创建服务器套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定套接字到端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        // 清空文件描述符集合
        FD_ZERO(&read_fds);

        // 将服务器套接字加入集合
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        // 将客户端套接字加入集合
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &read_fds);
            }
            if (client_sockets[i] > max_fd) {
                max_fd = client_sockets[i];
            }
        }

        // 使用 select 监视文件描述符
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select error");
            continue;
        }

        // 检查是否有新的连接
        if (FD_ISSET(server_fd, &read_fds)) {
            int addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept error");
                continue;
            }

            printf("New connection, socket fd: %d, IP: %s, Port: %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // 将新套接字加入客户端数组
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // 检查客户端套接字是否有数据可读, 遍历了每一个socket
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &read_fds)) {
                // 读取数据, buffer 在每次 read 时会被新数据覆盖，旧数据不会残留。
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // 客户端断开连接
                    socklen_t addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, &addrlen);
                    printf("Client disconnected, IP: %s, Port: %d\n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    close(sd);
                    client_sockets[i] = 0;  // 从数组中移除
                } else {
                    // 处理接收到的数据
                    buffer[valread] = '\0';  // 确保字符串以 null 结尾，避免读取到旧数据。
                    printf("Received from client %d: %s", sd, buffer);

                    // 回显数据给客户端
                    char response[80] = "Server: ";
                    std::strcat(response, buffer);
                    send(sd, response, strlen(response), 0);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}