/**
 * 用最小堆实现定时器
 */

#ifndef UNIXPROGRAMMING_TIME_HEAP_TIMER_H
#define UNIXPROGRAMMING_TIME_HEAP_TIMER_H

#include <iostream>
#include <netinet/in.h>
#include <ctime>

using std::exception;

#define BUFFER_SIZE 64

class heap_timer;

struct client_data {
    sockaddr_in addresss;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer *timer;
};

//定时器类
class heap_timer {
public:
    //定时器生效的绝对时间
    time_t expire;

    //回调函数
    void (*cb_func)(client_data *){};

    //数据
    client_data *user_data{};

    explicit heap_timer(int delay) { expire = time(nullptr) + delay; }
};

//时间堆类
class time_heap {
private:
    //堆数组
    heap_timer **array;
    //堆数组的容量
    int capacity;
    //堆数组当前包含的元素个数
    int cur_size;

    //最小堆的下虑操作，它确保堆数组中以第hole个节点作为根的子树拥有最小堆性质
    void percolate_down(int hole) {
        heap_timer *temp = array[hole];
        while (hole * 2 + 1 < cur_size - 1) {
            int child = hole * 2 + 1;
            //选出两个子节点中较小的一个
            if (child + 1 < cur_size - 1 && array[child + 1]->expire < array[child]->expire) ++child;
            //下移
            if (array[child]->expire < temp->expire) array[hole] = array[child];
            else break;

            hole = child;
        }
        //交换
        array[hole] = temp;
    }

    //将堆数组容量扩大1倍
    void resize() noexcept(false) {
        auto **temp = new heap_timer *[2 * capacity];
        for (int i = 0; i < 2 * capacity; ++i) temp[i] = nullptr;
        capacity = 2 * capacity;
        for (int i = 0; i < cur_size; ++i) temp[i] = array[i];
        delete[] array;
        array = temp;
    }

public:
    //初始化一个容量为cap的空堆
    explicit time_heap(int cap) noexcept(false): capacity(cap), cur_size(0) {
        //创建堆数组
        array = new heap_timer *[capacity];
        if (!array) throw std::exception();
        for (int i = 0; i < capacity; ++i) array[i] = nullptr;
    }

    //用已有的数组来初始化堆
    time_heap(heap_timer **init_array, int size, int capacity) noexcept(false): cur_size(size),
                                                                                capacity(capacity) {
        if (capacity < size) throw std::exception();
        //创建堆数组
        array = new heap_timer *[capacity];
        if (!array) throw std::exception();
        for (int i = 0; i < capacity; ++i) array[i] = nullptr;
        if (size != 0) {
            for (int i = 0; i < size; ++i) array[i] = init_array[i];
            //执行下虑调整操作
            for (int i = (cur_size - 1) / 2; i >= 0; --i) percolate_down(i);
        }
    }

    //销毁时间堆
    ~time_heap() {
        for (int i = 0; i < cur_size; ++i) delete array[i];
        delete[]array;
    }

    bool empty() const { return cur_size == 0; }

    //添加目标定时器timer
    void add_timer(heap_timer *timer) noexcept(false) {
        if (!timer) return;
        //扩容
        if (cur_size >= capacity) resize();

        int hole = ++cur_size;
        //对从空穴到根节点的路径上的所有节点执行上虑操作
        while (hole > 0) {
            int parent = (hole - 1) / 2;
            if (array[parent]->expire <= timer->expire) break;
            array[hole] = array[parent];
            hole = parent;
        }
        array[hole] = timer;
    }

    //删除目标定时器timer
    void del_timer(heap_timer *timer) {
        if (!timer) return;
        //仅仅将目标定时器的回调函数设置为空，即延迟销毁
        //将节省真正删除该定时器造成的开销，但容易让堆数组膨胀
        timer->cb_func = nullptr;
    }

    //获取堆顶部的定时器
    heap_timer *top() const {
        if (empty()) return nullptr;
        return array[0];
    }

    //删除堆顶部的定时器
    void pop_timer() {
        if (empty()) return;
        if (array[0]) {
            delete array[0];
            //将原来的堆顶元素替换为堆数组中最后一个元素
            array[0] = array[--cur_size];
            //下虑
            percolate_down(0);
        }
    }

    //心搏函数
    void tick() {
        heap_timer *tmp = array[0];
        //循环处理堆中到期的定时器
        time_t cur = time(nullptr);
        while (!empty()) {
            if (!tmp) break;
            //如果堆顶定时器没有到期
            if (tmp->expire > cur) break;
            //否则执行堆顶定时器中的任务
            if (array[0]->cb_func) array[0]->cb_func(array[0]->user_data);
            //将堆顶元素删除，同时生成新的堆顶定时器array[0]
            pop_timer();
            tmp = array[0];
        }
    }
};

#endif //UNIXPROGRAMMING_TIME_HEAP_TIMER_H
