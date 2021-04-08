/**
 * POSIX 命名信号量，需加-pthread链接选项
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


int main() {

    //创建信号量，可在/dev/shm下看到该文件
    sem_t* sp = sem_open("/posixsem", O_CREAT, O_RDWR, 1);

    pid_t pid = fork();
    if (pid < 0) return 1;
    else if (pid == 0) {
        //P操作
        sem_wait(sp);
        printf("child get this sem\n");
        sleep(5);
        //V操作
        sem_post(sp);
        exit(0);
    } else {
        //P操作
        sem_wait(sp);
        printf("parent get this sem\n");
        sleep(5);
        //V操作
        sem_post(sp);
    }
    
    waitpid(pid, NULL, 0);
    
    //关闭信号量
    sem_close(sp);
    //删除信号量
    sem_unlink("/posixsem");

    return 0;
}
