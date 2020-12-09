//cs::app,健壮的IO包
#include <unistd.h>
#include <errno.h>
#include <memory.h>

//不带缓冲,从fd读取至多n个字节到usrbuf,返回读取的字节数,在EOF时返回不足值,出错返回-1设置errno
//运行被系统中断,会自动重启
ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *) usrbuf;
    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            //中断就重启,非中断错误就返回-1,由read设置errno
            if (errno == EINTR) nread = 0;
            else return -1;
        } else if (nread == 0) break;  //遇到EOF
        nleft -= nread;
        bufp += nread;
    }
    return n - nleft;
}

//不带缓冲,从usrbuf向fd写入n个字节,返回写入的字节数,不会返回不足值，出粗返回-1设置errno
//运行被系统中断,会自动重启
ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char *) usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR) nwritten = 0;
            else return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}


//读缓冲区格式
#define RIO_BUFSIZE 8192
typedef struct {
    //要读取的文件描述符
    int rio_fd;
    //缓冲区剩余未读的字节数
    int rio_cnt;
    //下一个读取的位置
    char *rio_bufptr;
    //缓冲区
    char rio_buf[RIO_BUFSIZE];
} rio_t;

//读取前先要初始化
void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

//linux read的带缓冲版本，与之有相同的语义
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    //如果缓冲区为空，则重新填充
    while (rp->rio_cnt <= 0) {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        //非中断错误就返回-1，设置errno
        if (rp->rio_cnt < 0) { if (errno != EINTR) return -1; }
        else if (rp->rio_cnt == 0) return 0;    //遇到EOF
        else rp->rio_bufptr = rp->rio_buf;
    }
    //拷贝n和缓冲区字节数较小的
    int cnt = n;
    if (rp->rio_cnt < n) cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

//从rp读取下一个文本行(包括换行符)，最多读maxlen - 1个字节，以NULL结尾
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = (char *) usrbuf;
    for (n = 1; n < maxlen; ++n) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                ++n;
                break;
            }
        } else if (rc == 0) {   //EOF
            //没读过数据直接返回
            if (n == 1) return 0;
            else break;         //读过数据，还需要设置行末0
        } else return -1;       //errno由read设置
    }
    *bufp = 0;
    return n - 1;
}

//从rp最多读取最多n个字节
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *) usrbuf;

    while (nleft > 0) {
        //错误，errno被read设置
        if ((nread = rio_read(rp, bufp, nleft)) < 0) return -1;
        else if (nread == 0) break;     //EOF
        nleft -= nread;
        bufp += nread;
    }
    return n - nleft;
}
