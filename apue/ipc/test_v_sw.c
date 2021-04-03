/**
 * System V共享内存，写
 */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

int main() {

    //由路径名和项目id生成一个key
    key_t key = ftok("./test_v_sw.c", 0);

    //创建一个指定key的、0x400000字节的共享存储段
    int shm_id = shmget(key, 0x400000, IPC_CREAT | 0666);

    //映射共享存储段到虚拟地址空间，并返回该地址
    char* p = (char*)shmat(shm_id, NULL, 0);

    memset(p, 'A', 0x400000);

    //分离虚拟地址和共享存储段(不是删除)
    shmdt(p);

    return 0;
}
