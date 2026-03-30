//客户端代码，epoll版本
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define BUFFER_SIZE 1024

int main() {
    char ip_str[16];
    int port;
    printf("Enter Server IP: ");
    scanf("%s", ip_str);
    printf("Enter Server Port: ");
    scanf("%d", &port);
    while (getchar() != '\n');// 清空输入缓冲区

    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_str, &server_addr.sin_addr) <= 0) {
        printf("Invalid IP address format.\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server.\n");

    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[2];
    event.events = EPOLLIN;
    event.data.fd = 0;// 监控来自键盘的标准输入
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event);

    event.events = EPOLLIN;
    event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);//监控服务端的消息

    printf("Enter message (type 'quit' to exit): ");
    fflush(stdout);
    
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, 2, -1);
        for(int i = 0; i < num_events; i++){
            if (events[i].data.fd == 0) {// 处理标准输入
                if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                    if (strcmp(buffer, "quit\n") == 0) {
                        printf("Exiting the client.\n");
                        close(client_fd);
                        close(epoll_fd);
                        return 0;
                    }else{
                        buffer[strcspn(buffer, "\n")] = 0;
                        if (strlen(buffer)==0){continue;}
                        ssize_t send_len = send(client_fd, buffer, strlen(buffer), 0);
                        if (send_len < 0) {
                            perror("send failed");
                            close(client_fd);
                            close(epoll_fd);
                            return 0;
                        }
                    }
                }else{
                    perror("fgets failed");
                    close(client_fd);
                    close(epoll_fd);
                    return 0;
                }
            }else{// 处理服务器消息
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t recv_len = recv(client_fd, buffer, BUFFER_SIZE-1, 0);
                if (recv_len <= 0) {
                    if (recv_len < 0) {
                        perror("Recv failed");
                    } else {
                        printf("Server disconnected.\n");
                    }
                    close(client_fd);
                    close(epoll_fd);
                    return 0;
                }
                printf("Received from server: %s\n", buffer);
                printf("Enter message (type 'quit' to exit): ");
                fflush(stdout);
            }
        }

    }
    close(client_fd);
    printf("Client closed.\n");
    return 0;
}
