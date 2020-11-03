/**
 * 用epoll实现共用一个端口的TCP，UDP回射服务器
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/epoll.h>
#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
    struct sockaddr_in addresss;
    bzero(&addresss, sizeof(addresss));
    addresss.sin_family = AF_INET;
    addresss.sin_addr.s_addr = INADDR_ANY;
    addresss.sin_port = htons(port);

    //创建TCP socket并绑定到address
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (struct sockaddr *) &addresss, sizeof(addresss));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    //创建UDP socket并绑定到address
    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(udpfd >= 0);
    ret = bind(udpfd, (struct sockaddr *) &addresss, sizeof(addresss));
    assert(ret != -1);

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    //注册可读事件
    addfd(epollfd, listenfd);
    addfd(epollfd, udpfd);

    while (1) {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0) {
            printf("epoll failure\n");
            break;
        }

        for (int i = 0; i < number; ++i) {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *) &client_address, &client_addrlength);
                assert(connfd >= 0);
                addfd(epollfd, connfd);
            } else if (sockfd == udpfd) {
                char buf[UDP_BUFFER_SIZE];
                memset(buf, '\0', UDP_BUFFER_SIZE);
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                ret = recvfrom(udpfd, buf, UDP_BUFFER_SIZE - 1, 0, (struct sockaddr *) &client_address,
                               &client_addrlength);
                if (ret > 0) {
                    sendto(udpfd, buf, UDP_BUFFER_SIZE - 1, 0, (struct sockaddr *) &client_address, client_addrlength);
                }
            } else if (events[i].events & EPOLLIN) {
                char buf[TCP_BUFFER_SIZE];
                while (1) {
                    memset(buf, '\0', TCP_BUFFER_SIZE);
                    ret = recv(sockfd, buf, TCP_BUFFER_SIZE - 1, 0);
                    if (ret < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        close(sockfd);
                        break;
                    } else if (ret == 0) close(sockfd);
                    else send(sockfd, buf, ret, 0);
                }
            } else printf("something else happened\n");
        }
    }
    close(listenfd);
    close(udpfd);
    return 0;
}
