/**
 * POSIX共享内存，读，需加-lrt链接选项
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
    //只读打开共享文件，可在/dev/shm下看到该文件
    //名字必须以/开头
    int fd = shm_open("/posixsm", O_RDONLY, 0666);
    //截断
    ftruncate(fd, 0x400000);

    //映射
    char* p = mmap(NULL, 0x400000, PROT_READ, MAP_SHARED, fd, 0);

    printf("%c %c %c %c\n", p[0], p[1], p[2], p[3]);
    //解除映射
    munmap(p, 0x400000);

    //标记等待删除，当全部munmap时，删除
    shm_unlink("/posixsm");

    return 0;
}
