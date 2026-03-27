#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};
    ssize_t recv_len, send_len;

    fd_set master_fds, read_fds;    //master_fds是母本，记录所有的连接；read_fds是交给select修改的临时工
    int max_fd; // 记录当前最大的文件描述符数值
    
    
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
    
    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds); // 将server_fd添加到master_set中
    max_fd = server_fd; //

    while(1){
        read_fds = master_fds;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        } else if (activity == 0) {
            printf("select timeout\n");
        } else {
            for(int i = 0; i <= max_fd; i++ ){
                if(FD_ISSET(i,&read_fds)){
                    if(i==server_fd) // 处理新连接
                    {
                        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                        if (client_fd < 0) {
                            perror("accept failed");
                            continue;   // 继续循环，等待下一个连接
                        }
                        printf("Client connected from %s:%d, fd = %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
                        FD_SET(client_fd, &master_fds);
                        
                        if (client_fd > max_fd) {
                            max_fd = client_fd;
                        }
                    }else{ // 处理客户端的数据（i==client_fd）
                        memset(buffer, 0, BUFFER_SIZE);
                        recv_len = recv(i, buffer, BUFFER_SIZE - 1, 0);

                        if (recv_len <= 0) {
                            if (recv_len < 0) {
                                perror("recv failed");
                            } else {
                                printf("Client fd %d disconnected.\n", i);
                            }
                            close(i);   // 1. 关闭连接 
                            FD_CLR(i, &master_fds); // 2. 从母本中擦除
                            if (i == max_fd) {
                                while (max_fd > 0 && !FD_ISSET(max_fd, &master_fds)) {
                                    max_fd--;
                                } // 只有关掉的是当前最高标号，才需要重新找到最大的文件描述符
                            }

                        }else{ // receive成功，处理数据
                            char response[BUFFER_SIZE] = {0};
                            int prefix_len = snprintf(response, BUFFER_SIZE, "Echo from Server: %.*s", (int)recv_len, buffer);

                            send_len = send(i, response, prefix_len, 0);
                            if (send_len < 0) {
                                perror("send failed");
                                close(i);
                                FD_CLR(i, &master_fds);
                                if (i == max_fd) {
                                    while (max_fd > 0 && !FD_ISSET(max_fd, &master_fds)) {
                                        max_fd--;
                                    }
                                }
                            }else{
                                printf("Echo to client(fd %d): %s\n", i, buffer);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}