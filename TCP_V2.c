//Multithreaded Synchronous Version 多线程同步版本
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *handle_client(void *arg) {    // 处理客户端连接的函数
    int client_fd = *(int *)arg;    // 客户端文件描述符
    free(arg);    // 释放分配的内存（是main中malloc的client_fd_ptr）

    char buffer[BUFFER_SIZE] = {0};
    ssize_t recv_len, send_len;

    printf("[Thread %ld] Client connected.\n", pthread_self());
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv_len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (recv_len < 0) {
            perror("recv failed");
            break;
        } else if (recv_len == 0){  // 客户端主动断开连接（recv返回0），断开方法：在客户端先按ctrl+]，在输入quit回车
            printf("[Thread %ld] Client disconnected.\n", pthread_self());
            break;
        }

        char response[BUFFER_SIZE] = {0};
        int prefix_len = snprintf(response, BUFFER_SIZE, "Echo from Server: %.*s", (int)recv_len, buffer);

        send_len = send(client_fd, response, prefix_len, 0);
        if (send_len < 0) {
            perror("send failed");
            break;
        }
        printf("[Thread %ld]Echo to client: %s\n", pthread_self(), buffer);
    }

    close(client_fd);
    printf("[Thread %ld]Client connection closed.\n", pthread_self());

    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    pthread_t thread_id;
    
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

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { //绑定 socket 到端口
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Bound to port %d.\n", PORT);

    if (listen(server_fd, 3) < 0) { //监听连接，第一个参数为要监听的socket描述字，第二个参数为相应socket可以排队的最大连接个数。
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Listening for connections...\n");
    
    while(1){
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;   // 继续循环，等待下一个连接
        }
        printf("New Connection from %s:%d, creating thread...\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int* client_fd_ptr = (int*)malloc(sizeof(int));
        *client_fd_ptr = client_fd; //把client_fd的值赋给client_fd_ptr，避免client_fd_ptr在子线程中被修改
         if (pthread_create(&thread_id, NULL, handle_client, (void*)client_fd_ptr) != 0) {
            perror("pthread_create failed");
            close(client_fd);
            free(client_fd_ptr);
            continue;
        }
        
        pthread_detach(thread_id);//将线程设置为“分离状态”。这样当子线程运行结束时，系统会自动回收它的资源，不需要主线程去等待（join）它
        printf("Thread created successfully.\n");
    }
    close(server_fd);
    printf("Server socket closed.\n");
    return 0;
}