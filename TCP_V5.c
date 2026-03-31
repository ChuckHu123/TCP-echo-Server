//libevent version
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// Libevent 的核心头文件
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void event_cb(struct bufferevent *bev, short events, void *ctx){
    if(events & BEV_EVENT_CONNECTED){
        printf("Connection established!\n");
    } else if(events & BEV_EVENT_ERROR){
        perror("Connection error");
        bufferevent_free(bev);
    } else if(events & (BEV_EVENT_EOF)){
        printf("Client disconnected.\n");
        bufferevent_free(bev);
    }
}

void read_cb(struct bufferevent *bev, void *ctx){
    char buffer[BUFFER_SIZE];
    size_t len = bufferevent_read(bev, buffer, BUFFER_SIZE-1);
    buffer[len] = '\0';
    printf("Received data: %s\n", buffer);

    const char *reply = "Received data: ";
    bufferevent_write(bev, reply, strlen(reply));
    bufferevent_write(bev, buffer, len);
}

void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data){
    struct event_base *base = (struct event_base *)user_data;
    printf("New connection from %s:%d, fd = %d\n", 
        sa->sa_family == AF_INET ? inet_ntoa(((struct sockaddr_in *)sa)->sin_addr) : "unknown",
        sa->sa_family == AF_INET ? ntohs(((struct sockaddr_in *)sa)->sin_port) : 0,
        fd);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!\n");
        evutil_closesocket(fd);
        return;
    }

    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

int main() {
    signal(SIGPIPE, SIG_IGN);//忽略管道破裂错误，防止客户端异常断开导致服务器崩溃
    struct event_base *base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port=htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct evconnlistener *listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (!listener) {
        fprintf(stderr, "Could not create a listener!\n");
        return EXIT_FAILURE;
    }
    printf("Server listening on port %d...\n", PORT);

    int ret = event_base_dispatch(base);
    if (ret != 0) {
        fprintf(stderr, "Error in event_base_dispatch!\n");
        return EXIT_FAILURE;
    }

    evconnlistener_free(listener);
    event_base_free(base);
    printf("Server shutdown complete.\n");
    return 0;
}