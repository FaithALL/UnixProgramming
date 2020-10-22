/**
 * 用指数补偿法编写可移植的支持重试的connect
 */
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXSLEEP 128

int connect_retry(int domain, int type, int protocol,
                  const struct sockaddr *addr, socklen_t alen) {
    int numsec, fd;
    for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1) {
        //有的平台在同一个文件描述符重连会失败
        if ((fd = socket(domain, type, protocol)) < 0) return -1;
        if (connect(fd, addr, alen) == 0) return fd;
        close(fd);
        if (numsec <= MAXSLEEP / 2)
            sleep(numsec);
    }
    return -1;
}

