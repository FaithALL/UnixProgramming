/**
 * 测试sendfile函数，out_fd必须是socket fd, in_fd必须指向真实的文件
 * ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count)
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s port_number filename\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
    const char *file_name = argv[2];

    int filefd = open(file_name, O_RDONLY);
    assert(filefd >= 0);
    //获取文件属性
    struct stat stat_buf;
    fstat(filefd, &stat_buf);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr *) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_length = sizeof(client);
    int connfd = accept(sock, (struct sockaddr *) &client, &client_length);
    if (connfd < 0) printf("errno is %d\n", errno);
    else {
        sendfile(connfd, filefd, NULL, stat_buf.st_size);
        close(connfd);
    }
    close(sock);
    return 0;
}

