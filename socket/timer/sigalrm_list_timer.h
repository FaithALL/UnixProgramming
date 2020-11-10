/**
 * 使用SIGALRM信号实现升序链表定时器，以超时时间排序
 */

#ifndef UNIXPROGRAMMING_SIGALRM_LIST_TIMER_H
#define UNIXPROGRAMMING_SIGALRM_LIST_TIMER_H

#include <ctime>
#include <arpa/inet.h>
#include <cstdio>

#define BUFFER_SIZE 64

class util_timer;

//用户数据结构：客户端socket地址、socket文件描述符、读缓存和定时器
struct client_data {
    struct sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

//定时器类，双向链表的结点
class util_timer {
public:
    //任务的超时时间，使用绝对时间
    time_t expire = 0;

    //任务回调函数
    void (*cb_func)(client_data *){};

    //回调函数处理的客户数据，由定时器的执行者传递给回调函数
    client_data *user_data = nullptr;
    util_timer *prev = nullptr;
    util_timer *next = nullptr;

    util_timer() = default;
};

//定时器链表类，是一个升序、双向链表，带有头结点和尾结点
class sort_timer_list {
private:
    util_timer *head = nullptr;
    util_timer *tail = nullptr;

    //将目标定时器timer添加到结点lst_head之后的部分链表
    void add_timer(util_timer *timer, util_timer *lst_head) {
        util_timer *prev = lst_head;
        util_timer *tmp = lst_head->next;
        //找到第一个超时时间大于目标定时器的超时时间的节点，并将目标定时器插入该节点之前
        while (tmp) {
            if (timer->expire < tmp->expire) {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        //超时时间最大的一个
        if (!tmp) {
            prev->next = timer;
            timer->prev = prev;
            timer->next = nullptr;
            tail = timer;
        }
    }

public:
    sort_timer_list() = default;

    //销毁链表
    ~sort_timer_list() {
        util_timer *tmp = head;
        while (tmp) {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }

    //将目标定时器加入链表,升序
    void add_timer(util_timer *timer) {
        if (!timer) return;
        if (!head) {
            head = tail = timer;
            return;
        }
        if (timer->expire < head->expire) {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);
    }

    //当设置某个定时任务超时延长时，定时器需要往链表的尾部移动
    void adjust_timer(util_timer *timer) {
        if (!timer) return;
        util_timer *tmp = timer->next;
        //如果被调整的目标定时器处在链表尾部，或者该定时器超时值仍然小于其下一个定时器的超时值，不用调整
        if (!tmp || (timer->expire < tmp->expire)) return;

        //若是头节点，重新插入
        if (timer == head) {
            head = head->next;
            head->prev = nullptr;
            timer->next = nullptr;
            add_timer(timer, head);
        } else {
            //先取出
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            //再插入
            add_timer(timer, timer->next);
        }
    }

    //将目标定时器timer从链表中删除
    void del_timer(util_timer *timer) {
        if (!timer) return;
        //如果只有一个定时器
        if ((timer == head) && (timer == tail)) {
            delete timer;
            tail = head = nullptr;
            return;
        }
        //目标是头
        if (timer == head) {
            head = head->next;
            head->prev = nullptr;
            delete timer;
            return;
        }
        //目标是尾
        if (timer == tail) {
            tail = tail->prev;
            tail->next = nullptr;
            delete timer;
            return;
        }
        //一般情况
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    //SIGALRM信号每次被触发就在其信号处理函数中执行一次，tick函数，以处理链表上到期的任务
    void tick() {
        if (!head) return;
        printf("timer tick\n");
        //获取当前系统时间
        time_t cur = time(nullptr);
        util_timer *tmp = head;
        //从头到尾处理每个定时器，直到遇到一个尚未到期的定时器
        while (tmp) {
            //每个定时器采用绝对时间作为超时值，我们可以把定时器的超时时间和系统当前时间比较，以判断是否到期
            if (cur < tmp->expire) break;
            //执行回调函数
            tmp->cb_func(tmp->user_data);
            //执行完后删除，重置头结点
            head = tmp->next;
            if (head) head->prev = nullptr;
            delete tmp;
            tmp = head;
        }
    }
};

#endif //UNIXPROGRAMMING_SIGALRM_LIST_TIMER_H
