//客户端代码，半双工只能一问一答
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main() {
    char ip_str[16]; // 用于存放 IP，如 "127.0.0.1"
    int port; // 用于存放端口号，如 8080
    printf("Enter Server IP: ");
    scanf("%s", ip_str);
    printf("Enter Server Port: ");
    scanf("%d", &port);
    while (getchar() != '\n'); // 清空输入缓冲区的\n, 确保后续输入正确

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

    if (inet_pton(AF_INET, ip_str, &server_addr.sin_addr) <= 0) { //将字符串形式的 IP 转换为二进制网络字节序
        printf("Invalid IP address format.\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { // 尝试连接服务器
        perror("Connection Failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server.\n");

    while (1) {
        printf("Enter message (type 'quit' to exit): ");

        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL){ // 从标准输入读取数据
            if (strncmp(buffer, "quit", 4) == 0){
                printf("Exiting the client.\n");
                break;
            }
            
            buffer[strcspn(buffer, "\n")] = 0; // 去掉输入缓冲区的换行符
            if (strlen(buffer) == 0) {
                continue; // 如果是空行，重新开始 while 循环，不要去 recv
            
            }
            ssize_t send_len = send(client_fd, buffer, strlen(buffer), 0);
            if (send_len < 0) {
                perror("send failed");
                break;
            }
            printf("Message sent: %s\n", buffer);

            memset(buffer, 0, BUFFER_SIZE);
            ssize_t recv_len = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if (recv_len <= 0) {
                if (recv_len < 0) {
                    perror("Recv failed");
                } else {
                    printf("Server disconnected.\n");
                }
                break;
            }
            printf("Received from server: %s\n", buffer);

        }else{
            perror("fgets failed");
            break;
        }

    }
    close(client_fd);
    printf("Client closed.\n");
    return 0;
}
