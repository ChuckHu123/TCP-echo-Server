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
    /*
    int socket(int domain, int type, int protocol);
        domain：即协议域，决定了socket的地址类型AF_INET:IPv4
        type：指定socket类型。SOCK_STREAM（流式套接字）
        protocol：就是指定协议，为0时，会自动选择type类型对应的默认协议
    */
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    /*
    struct sockaddr_in {
        sa_family_t    sin_family; / address family: AF_INET 
        in_port_t      sin_port;   / port in network byte order
        struct in_addr sin_addr;   / internet address
        };

    struct in_addr {
        uint32_t       s_addr;     / address in network byte order 
        };
    */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { //绑定 socket 到端口
        // 因为bind /connect/accept 这些系统函数是通用的，所以系统设计了一个通用类型 struct sockaddr，让所有协议都能兼容
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Bound to port %d.\n", PORT);
    /*struct sockaddr{  //sockaddr结构体和sockaddr_in结构体都是地址族+端口+IP地址的结构体，可以相互转换
        sa_family_t  sin_family;   //地址族（Address Family），也就是地址类型
        char         sa_data[14];  //IP地址和端口号
        };
    */

    if (listen(server_fd, 1) < 0) { //监听连接，第一个参数为要监听的socket描述字，第二个参数为相应socket可以排队的最大连接个数。
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Listening for connections...\n");

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    //inet_ntoa()将二进制的IP地址转换为点分十进制的字符串，ntohs()将网络字节序的端口号（大端序）转换为主机字节序的端口号（小端序）

    printf("How to disconnect from server: Ctrl+] then type 'quit' and press Enter.\n");
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv_len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        /*
        第一个参数指定发送端套接字描述符；
        第二个参数指明一个存放应用程序要发送数据的缓冲区；
        第三个参数指明实际要发送的数据的字节数；留1位存'\0'，避免字符串越界
        第四个参数一般置0
        */
        if (recv_len < 0) {
            perror("recv failed");
            break;
        } else if (recv_len == 0){  // 客户端主动断开连接（recv返回0），断开方法：在客户端先按ctrl+]，在输入quit回车
            printf("Client disconnected.\n");
            break;
        }

        send_len = send(client_fd, buffer, recv_len, 0);
        if (send_len < 0) {
            perror("send failed");
            break;
        }
        printf("Echo to client: %s\n", buffer);
    }

    close(client_fd);
    printf("Client connection closed.\n");

    close(server_fd);
    printf("Server socket closed. Exiting.\n");

    return 0;//test for git
    //test2
}