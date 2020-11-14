/**
 * 基于哈希思想改进升序链表计时器为时间轮
 */
#ifndef UNIXPROGRAMMING_TIME_WHEEL_TIMER_H
#define UNIXPROGRAMMING_TIME_WHEEL_TIMER_H

#include <ctime>
#include <cstdio>
#include <netinet/in.h>

#define BUFFER_SIZE 64

class tw_timer;

struct client_data {
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
};

//定时器类
class tw_timer {
public:
    //定时器在时间轮转多少圈后生效
    int rotation;
    //定时器属于时间轮上哪个槽
    int time_slot;

    //回调函数
    void (*cb_func)(client_data *){};

    //指向下一个定时器
    tw_timer *next = nullptr;
    //指向前一个定时器
    tw_timer *prev = nullptr;

    //数据
    client_data *user_data{};

    tw_timer(int rot, int ts)
            : rotation(rot), time_slot(ts) {}
};

class time_wheel {
private:
    //时间轮上槽的数目
    static const int N = 60;
    //每1s时间轮转动一次，即槽间隔(slot interval)为1s
    static const int SI = 1;
    //时间轮的槽，每个元素为一个无序定时器链表头节点
    tw_timer *slots[N]{nullptr};
    //时间轮当前槽
    int cur_slot = 0;
public:
    time_wheel() = default;

    ~time_wheel() {
        for (auto &slot : slots) {
            tw_timer *tmp = slot;
            while (tmp) {
                slot = tmp->next;
                delete tmp;
                tmp = slot;
            }
        }
    }

    //根据定时值timeout创建一个定时器，并把它插入合适的槽中
    tw_timer *add_timer(int timeout) {
        if (timeout < 0) return nullptr;
        int ticks;
        //根据待插入定时器的超时值计算它将在时间轮转动多少个滴答后被触发，并将该滴答数存储于变量ticks中
        //待插入定时器的超时值小于时间轮的槽间隔SI，则将ticks向上折合为1，否则就将ticks向下折合为timeout/SI
        if (timeout < SI) ticks = 1;
        else ticks = timeout / SI;
        //计算待插入的定时器在时间轮转动多少圈后被触发
        int rotation = ticks / N;
        //计算待插入的定时器应该被插入哪个槽中
        int ts = (cur_slot + (ticks % N)) % N;
        //创建新的定时器，它在时间轮转动rotation圈之后被触发，且位于第ts个槽上
        auto *timer = new tw_timer(rotation, ts);

        //如果第ts个槽中尚无任何定时器，则把新建的定时器插入其中，并将该定时器设置为该槽的头节点
        if (!slots[ts]) {
            printf("add timer, rotaton is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot);
            slots[ts] = timer;
        } else {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }

    //删除目标定时器timer
    void del_timer(tw_timer *timer) {
        if (!timer) return;
        int ts = timer->time_slot;
        //slots[ts]是目标定时器所在槽的头节点，如果目标定时器就是该头节点，则需要重置第ts个槽的头节点
        if (timer == slots[ts]) {
            slots[ts] = slots[ts]->next;
            if (slots[ts]) slots[ts]->prev = nullptr;
            delete timer;
        } else {
            timer->prev->next = timer->next;
            if (timer->next) timer->next->prev = timer->prev;
            delete timer;
        }
    }

    //SI时间到后，调用该函数，时间轮向前滚动一个槽的间隔
    void tick() {
        //取得时间轮上当前槽的头节点
        tw_timer *tmp = slots[cur_slot];
        printf("current slot is %d\n", cur_slot);
        while (tmp) {
            printf("tick the timer once\n");
            //如果定时器的rotation值大于0，则它在这一轮不起作用
            if (tmp->rotation > 0) {
                --(tmp->rotation);
                tmp = tmp->next;
            } else {
                tmp->cb_func(tmp->user_data);
                if (tmp == slots[cur_slot]) {
                    printf("delete header in cur_slot\n");
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if (slots[cur_slot]) slots[cur_slot]->prev = nullptr;
                    tmp = slots[cur_slot];
                } else {
                    tmp->prev->next = tmp->next;
                    if (tmp->next) tmp->next->prev = tmp->prev;
                    tw_timer *tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot = (cur_slot + 1) % N;
    }
};

#endif //UNIXPROGRAMMING_TIME_WHEEL_TIMER_H
