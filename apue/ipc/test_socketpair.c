/**
 * 使用socketpair传递文件描述符
 * 《linux高性能服务器编程》P328
 */

#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

void send_fd(int fd, int fd_to_send){

    struct msghdr msg;
    struct iovec iov[1];
    char buf[1];

    iov[0].iov_base =  buf;
    iov[0].iov_len = 1;
    //不关心地址，由fd就可以确定接收方
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    struct cmsghdr cm;
    //发送文件描述符的几个关键步骤
    //CMSG_LEN(unsigned int nbytes)返回为存储nbytes字节的数据对象分配的cmsghdr字节数
    cm.cmsg_len = CMSG_LEN(sizeof(int));
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    *(int*)CMSG_DATA(&cm) = fd_to_send;
    msg.msg_control = &cm;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    sendmsg(fd, &msg, 0);
}

int recv_fd(int fd) {

    struct msghdr msg;
    struct iovec iov[1];
    char buf[1];

    iov[0].iov_base =  buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    struct cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    recvmsg(fd, &msg, 0);

    int fd_to_send = *(int*)CMSG_DATA(&cm);
    return fd_to_send;
}

int main(){
    int pipefd[2];
    int fd_to_send;
    socketpair(AF_UNIX, SOCK_DGRAM, 0, pipefd);

    pid_t pid = fork();
    //子进程发送文件描述符给父进程
    if (pid == 0) {
        close(pipefd[0]);
        fd_to_send = open("test.txt", O_RDWR, 0666);
        assert(fd_to_send >= 0);
        send_fd(pipefd[1], fd_to_send);
        close(fd_to_send);
        exit(0);
    }

    close(pipefd[1]);
    fd_to_send = recv_fd(pipefd[0]);
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    read(fd_to_send, buf, 1024);
    printf("I got fd %d and data %s\n", fd_to_send, buf);
    close(fd_to_send);
}
