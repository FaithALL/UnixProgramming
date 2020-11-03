/**
 * 用poll实现聊天室客户端，同时处理网络连接和用户输入
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 64

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    if (connect(sockfd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("connection failed\n");
        close(sockfd);
        return 1;
    }

    struct pollfd fds[2];
    //注册文件描述符0(标准输入)和文件描述符sockfd上的可读事件
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    //POLLRDHUP tcp连接被对方关闭或对方关闭读端
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);

    while (1) {
        ret = poll(fds, 2, -1);
        if (ret < 0) {
            printf("poll failure\n");
            break;
        }

        if (fds[1].revents & POLLRDHUP) {
            printf("server close the connection\n");
            break;
        } else if (fds[1].revents & POLLIN) {
            memset(read_buf, '\0', BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            printf("%s\n", read_buf);
        }

        if (fds[0].revents & POLLIN) {
            //使用splice将用户输入的数据直接写到sockfd上，zero-copy
            ret = splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            assert(ret != -1);
            ret = splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            assert(ret != -1);
        }
    }

    close(sockfd);
    return 0;
}
