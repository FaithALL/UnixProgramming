/**
 * POSIX共享内存，写，需加-lrt链接选项
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(){
    //打开/创建共享内存，可在/dev/shm下看到该文件
    //名字必须以/开头
    int fd = shm_open("/posixsm", O_CREAT | O_RDWR, 0666);
    //截断
    ftruncate(fd, 0x400000);

    //映射
    char* p = mmap(NULL, 0x400000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    memset(p, 'A', 0x400000);
    //解除映射
    munmap(p, 0x400000);

    return 0;
}
