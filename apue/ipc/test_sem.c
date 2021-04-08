/**
 * System V 信号量(集)
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdlib.h>

//如果要用semctl第4个参数，必须显式声明该类型
union semun{
    int                 val;
    struct semid_ds*    buf;
    unsigned short*    array;
    struct seminfo*     _buf;
};

//对信号量sem_id获取或释放op数目的资源
//op为-1是p，op为1是v
void pv(int sem_id, int op){
    struct sembuf sem_b;
    //指定操作的信号量编号
    sem_b.sem_num = 0;
    //信号量空余资源
    sem_b.sem_op = op;
    //在进程终止时撤销该操作
    sem_b.sem_flg = SEM_UNDO;

    semop(sem_id, &sem_b, 1);
}

int main() {

	//获取或创建一个匿名，信号量数目为1的信号量集
    int sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    union semun sem_un;
    sem_un.val = 1;
    //操作信号量集中第0个信号量，设置其初始值为sem_un.val
    semctl(sem_id, 0, SETVAL, sem_un);

    pid_t pid = fork();
    if (pid < 0) return 1;
    else if (pid == 0) {
        //P操作
        pv(sem_id, -1);
        printf("child get this sem\n");
        sleep(5);
        //V操作
        pv(sem_id, 1);
        exit(0);
    } else {
        //P操作
        pv(sem_id, -1);
        printf("parent get this sem\n");
        sleep(5);
        //V操作
        pv(sem_id, 1);
    }

    waitpid(pid, NULL, 0);
    semctl(sem_id, 0, IPC_RMID, sem_un,1);
    return 0;
}
