/**
 * 学习IO复用，EPOLLONESHOT事件（设置后，某事件最多触发一次，用于防止多个线程同时操作一个socket）
 */

#include <stdio.h>
#include <libgen.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1024

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

struct fds {
    int epollfd;
    int sockfd;
};

//将fd上的EPOLLIN和EPOLLET事件注册到epollfd指示的epoll内核事件表中
//参数oneshot指定是否注册fd上的EPOLLONESHOT事件
void addfd(int epollfd, int fd, bool oneshot) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (oneshot) event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//重置fd上的事件。这样操作后，尽管fd上的EPOLLONESHOT事件被注册，但是操作系统仍会触发fd上的EPOLLIN事件，且只触发一次
void reset_oneshot(int epollfd, int fd) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

//工作线程
void *worker(void *arg) {
    int sockfd = ((struct fds *) arg)->sockfd;
    int epollfd = ((struct fds *) arg)->epollfd;
    printf("start new thread to receive data on fd : %d\n", sockfd);
    char buf[BUFFER_SIZE];
    memset(buf, '\0', BUFFER_SIZE);
    //循环读取sockfd上的数据，直到遇到EAGAIN错误
    while (1) {
        int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
        if (ret == 0) {
            close(sockfd);
            printf("foreiner closed the connection\n");
            break;
        } else if (ret < 0) {
            if (errno == EAGAIN) {
                reset_oneshot(epollfd, sockfd);
                printf("read later\n");
                break;
            }
        } else {
            printf("get content: %s\n", buf);
            //休眠5秒，模拟数据处理
            sleep(5);
        }
    }

    printf("end thread receiving data on fd: %d\n", sockfd);
    return NULL;
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
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    //注意监听socket listenfd上是不能注册EPOLLONESHOT事件的，否则应用程序只能处理一个客户连接
    //因为后续的客户连接请求将不再触发listenfd上的EPOLLIN事件
    addfd(epollfd, listenfd, false);

    while (1) {
        int cnt = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (cnt < 0) {
            printf("epoll failure\n");
            break;
        }

        for (int i = 0; i < cnt; ++i) {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *) &client_address, &client_addrlength);
                //对非监听文件描述符都注册EPOLLONESHOT事件
                addfd(epollfd, connfd, true);
            } else if (events[i].events & EPOLLIN) {
                pthread_t thread;
                struct fds fds_for_new_worker;
                fds_for_new_worker.epollfd = epollfd;
                fds_for_new_worker.sockfd = sockfd;
                //新启动一个工作线程为sockfd服务
                pthread_create(&thread, NULL, worker, (void *) &fds_for_new_worker);
            } else {
                printf("something else happened\n");
            }
        }
    }
    close(listenfd);
    return 0;
}