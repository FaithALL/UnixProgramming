//利用pthread_sigmask和sigwait实现在一个线程处理所有的信号
//sigwait专门用于处理被挂起的信号
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define handle_error_en(en, msg) \
    do {errno = en; perror(msg); exit(EXIT_FAILURE); } while(0)

static void *sig_thread(void *arg) {
    sigset_t *set = (sigset_t *) arg;
    int s, sig;
    while (1) {
        //第二个步骤，调用sigwait等待信号
        s = sigwait(set, &sig);
        if (s != 0) handle_error_en(s, "sigwait");
        printf("Signal handling thread got signal %d\n", sig);
    }
}

int main() {
    pthread_t thread;
    sigset_t set;
    int s;

    //第一步骤，在主线程中设置信号掩码
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    s = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (s != 0) handle_error_en(s, "pthread_sigmask");
    s = pthread_create(&thread, NULL, &sig_thread, (void *) &set);
    if (s != 0) handle_error_en(s, "pthread_create");
    pause();
}