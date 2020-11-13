/**
 * 使用升序链表定时器处理非活动连接
 */

#include "sigalrm_list_timer.h"
#include <fcntl.h>
#include <cstring>
#include <cassert>
#include <csignal>
#include <cerrno>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>

#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
#define TIMESLOT 5
static int pipefd[2];

//利用升序链表管理定时器
static sort_timer_list timer_lst;
static int epollfd = 0;

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd) {
    epoll_event event{};
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void sig_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *) &msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig) {
    struct sigaction sa{};
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void timer_handler() {
    //定时处理任务
    timer_lst.tick();
    //每次alarm触发一次SIGALRM信号，所以要重新定时，以不断触发SIGALRM信号
    alarm(TIMESLOT);
}

//定时器回调函数，它删除非活动连接socket上的注册事件，并关闭
void cb_func(client_data *user_data) {
    assert(user_data);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, nullptr);
    close(user_data->sockfd);
    printf("close fd %d\n", user_data->sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);

    struct sockaddr_in address{};
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

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd);

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0]);

    //设置信号处理函数
    addsig(SIGALRM);
    addsig(SIGTERM);
    bool stop_server = false;

    auto *users = new client_data[FD_LIMIT];
    bool timeout = false;
    alarm(TIMESLOT);

    while (!stop_server) {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i < number; ++i) {
            int sockfd = events[i].data.fd;
            //处理客户连接
            if (sockfd == listenfd) {
                struct sockaddr_in client_address{};
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *) &client_address, &client_addrlength);
                addfd(epollfd, connfd);
                users[connfd].address = client_address;
                users[connfd].sockfd = connfd;
                //创建定时器,设置其回调函数和超时时间,然后绑定定时器与用户数据,最后将定时器添加到链表timer_lst中
                auto *timer = new util_timer;
                timer->user_data = &users[connfd];
                timer->cb_func = cb_func;
                time_t cur = time(nullptr);
                timer->expire = cur + 3 * TIMESLOT;
                users[connfd].timer = timer;
                timer_lst.add_timer(timer);
            } else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)) {
                //处理信号
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if (ret <= 0) continue;
                else {
                    for (int j = 0; j < ret; ++j) {
                        switch (signals[j]) {
                            case SIGALRM:
                                //用timeout变量标记有定时任务需要处理,但不立即处理,因为定时任务的优先级不高
                                timeout = true;
                                break;
                            case SIGTERM:
                                stop_server = true;
                        }
                    }
                }
            } else if (events[i].events & EPOLLIN) {
                //处理客户连接上的数据
                memset(users[sockfd].buf, '\0', BUFFER_SIZE);
                ret = recv(sockfd, users[sockfd].buf, BUFFER_SIZE - 1, 0);
                printf("get %d byte of client data %s from %d\n", ret, users[sockfd].buf, sockfd);
                util_timer *timer = users[sockfd].timer;
                if (ret < 0) {
                    //如果发生读错误,关闭连接,并移除其对应的定时器
                    if (errno != EAGAIN) {
                        cb_func(&users[sockfd]);
                        if (timer) timer_lst.del_timer(timer);
                    }
                } else if (ret == 0) {
                    //对方关闭连接,服务端也关闭连接,移除定时器
                    cb_func(&users[sockfd]);
                    if (timer) timer_lst.del_timer(timer);
                } else {
                    //某个客户连接上有数据可读,则调整该连接的定时器,以延迟该连接被关闭的时间
                    if (timer) {
                        time_t cur = time(nullptr);
                        timer->expire = cur + 3 * TIMESLOT;
                        printf("adjust timer once\n");
                        timer_lst.adjust_timer(timer);
                    } else {
                        //others
                    }
                }
            }
        }
        if (timeout) {
            timer_handler();
            timeout = false;
        }
    }

    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete[]users;
    return 0;
}
