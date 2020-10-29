/**
 * 通过writev发送http响应
 */

#include <stdio.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
//定义http状态码和状态信息

static const char *status_line[2] = {"200 OK", "500 Internal server error"};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s port_number filename\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
    const char *file_name = argv[2];

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr *) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);

    int connfd = accept(sock, (struct sockaddr *) &client, &client_addrlength);
    if (connfd < 0) printf("errno is : %d\n", errno);
    else {
        //保存HTTP应答的状态行、头部字段和空行
        char header_buf[BUFFER_SIZE];
        memset(header_buf, '\0', BUFFER_SIZE);
        //存放请求文件的内容
        char *file_buf = NULL;
        //存放目标文件的stat
        struct stat file_stat;
        //确定文件是否有效
        bool valid = true;
        //记录header_buf已用空间
        int len = 0;
        //文件不存在
        if (stat(file_name, &file_stat) < 0) valid = false;
        else {
            //是一个普通文件且具有读权限
            if (S_ISREG(file_stat.st_mode) && (file_stat.st_mode & S_IROTH)) {
                int fd = open(file_name, O_RDONLY);
                file_buf = malloc(file_stat.st_size + 1);
                memset(file_buf, '\0', file_stat.st_size + 1);
                if (read(fd, file_buf, file_stat.st_size) < 0) valid = false;
            } else valid = false;
        }

        //文件成功读入,则附加文件内容
        if (valid) {
            //状态栏
            ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[0]);
            len += ret;
            //头部1
            ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "Content-Length: %ld\r\n", file_stat.st_size);
            len += ret;
            snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");

            struct iovec iv[2];
            iv[0].iov_base = header_buf;
            iv[0].iov_len = strlen(header_buf);
            iv[1].iov_base = file_buf;
            iv[1].iov_len = strlen(file_buf);
            writev(connfd, iv, 2);
        } else {
            ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[1]);
            len += ret;
            ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");
            send(connfd, header_buf, ret, 0);
        }
        close(connfd);
        if (file_buf != NULL) free(file_buf);
    }
    close(sock);
    return 0;
}
