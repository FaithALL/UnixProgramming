/**
 * System V共享内存，读
 */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>

int main() {

    //由路径名和项目id生成一个key
    key_t key = ftok("./test_v_sw.c", 0);

    //获取共享存储段，而不是创建(记得ipcrm删除)
    int shm_id = shmget(key, 0x400000, 0666);

    //映射共享存储段到虚拟地址空间，并返回该地址
    char* p = (char*)shmat(shm_id, NULL, 0);

    printf("%c %c %c %c\n", p[0], p[1], p[2], p[3]);

    //分离虚拟地址和共享存储段
    shmdt(p);

    return 0;
}
