//epoll I/O Multiplexing Version 基于 epoll 的 I/O 多路复用版本
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/epoll.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};
    ssize_t recv_len, send_len;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //创建 socket
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Bound to port %d.\n", PORT);

    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Listening for connections...\n");
    
    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[MAX_EVENTS]; //events用来存储epoll_wait返回的事件
    event.events = EPOLLIN; // 监听读事件
    event.data.fd = server_fd; // 关联服务器 fd
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        perror("epoll_ctl failed");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    while(1){
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); //等待事件发生
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd < 0) {
                    perror("accept failed");
                    continue;
                }
                printf("Client connected from %s:%d, fd = %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
                event.events = EPOLLIN;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {// 添加客户端 fd 到 epoll 中
                    perror("epoll_ctl failed");
                    close(client_fd);
                    continue;
                }
            } else {
                memset(buffer, 0, BUFFER_SIZE);
                recv_len = recv(events[i].data.fd, buffer, BUFFER_SIZE - 1, 0);
                if (recv_len <= 0) {
                    if (recv_len < 0) {
                        perror("recv failed");
                    } else {
                        printf("Client fd %d disconnected.\n", events[i].data.fd);
                    }
                    close(events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                } else {
                    printf("Received from client(fd %d): %s\n", events[i].data.fd, buffer);
                    send_len = send(events[i].data.fd, buffer, recv_len, 0);
                    if (send_len < 0) {
                        perror("send failed");
                        close(events[i].data.fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    }
                }
            }
        }
    }
    return 0;
}