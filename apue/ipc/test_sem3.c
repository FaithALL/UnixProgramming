/**
 * POSIX 匿名信号量，需加-pthread链接选项，可在进程间或线程间使用
 * 对进程间同步，匿名信号量必须放在共享区域(mmap、shm_open、shmget)
 * 对线程间不同，匿名信号量必须是全局变量或者放在堆上
 * 具体见《TLPI》904页
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

sem_t sp;

void* threadFunc(void* arg) {
    sleep(1);
    //P操作
    sem_wait(&sp);
    printf("%s\n", (char*)arg);
    sleep(3);
    //V操作
    sem_post(&sp);
    return NULL;
}

int main() {

    //初始化匿名信号量，0表示线程间使用，1表示信号量的初值
    sem_init(&sp, 0, 1);

    //父子线程执行threadFunc
    pthread_t tid;
    pthread_create(&tid, NULL, threadFunc, "child thread get this sem");
    threadFunc("parent thread get this sem");

    pthread_join(tid, NULL);

    //销毁匿名信号量
    sem_destroy(&sp);

    return 0;
}
