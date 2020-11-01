/**
 * 学习IO复用epoll，使用LT和ET两种模式
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

//设置文件描述符为阻塞的
int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中，参数enable_et指定是否对fd启用ET模式
void addfd(int epollfd, int fd, bool enable_et) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et) event.events |= EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//LT模式的工作流程
void lt(struct epoll_event *events, int number, int epollfd, int listenfd) {
    char buf[BUFFER_SIZE];
    for (int i = 0; i < number; ++i) {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd) {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listenfd, (struct sockaddr *) &client_address, &client_addrlength);
            //将connfd可读事件注册到epoll事件表
            addfd(epollfd, connfd, false);
        } else if (events[i].events & EPOLLIN) {
            //只要socket读缓冲区中还有未读出数据，这段代码就会被出发
            printf("event trigger once\n");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if (ret <= 0) {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content: %s\n", ret, buf);
        } else printf("something else happend \n");
    }
}

//et模式的工作流程
void et(struct epoll_event *event, int number, int epollfd, int listenfd) {
    char buf[BUFFER_SIZE];
    for (int i = 0; i < number; ++i) {
        int sockfd = event[i].data.fd;
        if (sockfd == listenfd) {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listenfd, (struct sockaddr *) &client_address, &client_addrlength);
            addfd(epollfd, connfd, true);
        } else if (event[i].events & EPOLLIN) {
            //这段代码不会被重复触发，所以一次读完所有数据
            printf("event trigger once\n");
            while (1) {
                memset(buf, '\0', BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if (ret < 0) {
                    //对于非阻塞IO，下面条件成立表示数据已经全部读取完毕，
                    // 此后epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                } else if (ret == 0) close(sockfd);
                else printf("get %d bytes of content: %s\n", ret, buf);
            }
        } else printf("something else happened\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port_number\n", basename(argv[0]));
        return 1;
    }
    int port = atoi(argv[1]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (struct sockaddr *) &address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd, true);

    while (1) {
        int cnt = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (cnt < 0) {
            printf("epoll failuret \n");
            break;
        }
//        lt(events, cnt, epollfd, listenfd);
        et(events, cnt, epollfd, listenfd);
    }
    close(listenfd);
    return 0;
}
