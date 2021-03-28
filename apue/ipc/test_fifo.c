/**
 * 有名管道在阻塞模式下，只有读写两端都被打开(一般是不同进程)，才不被阻塞
 * 非阻塞模式下，读正常返回，写出错并设置errno
 */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * 假设已经存在fifo文件fifo_test，且另一端会进行写
 */
int main() {
    int fd = open("fifo_test", O_RDONLY);
    int buf[50];
    int bytes = read(fd, buf, sizeof(buf));
    write(STDOUT_FILENO, buf, bytes);
    return 0;
}
