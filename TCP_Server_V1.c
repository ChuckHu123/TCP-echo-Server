// Single Client Synchronous Version单客户端同步版本
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

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

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { //绑定 socket 到端口
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Bound to port %d.\n", PORT);

    if (listen(server_fd, 1) < 0) { //监听连接，第一个参数为要监听的socket描述字，第二个参数为相应socket可以排队的最大连接个数。
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
        printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        printf("How to disconnect from server: Ctrl+] then type 'quit' and press Enter.\n");
        
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            recv_len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

            if (recv_len < 0) {
                perror("recv failed");
                break;
            } else if (recv_len == 0){  // 客户端主动断开连接（recv返回0），断开方法：在客户端先按ctrl+]，在输入quit回车
                printf("Client disconnected.\n");
                break;
            }

            char response[BUFFER_SIZE] = {0};
            int prefix_len = snprintf(response, BUFFER_SIZE, "Echo from Server: %.*s", (int)recv_len, buffer);//优化客户端返回内容

            send_len = send(client_fd, response, prefix_len, 0);
            if (send_len < 0) {
                perror("send failed");
                break;
            }
            printf("Echo to client: %s\n", buffer);
        }
        close(client_fd);
        printf("Client connection closed. Ready for next connection...\n");
    }
    return 0;
}