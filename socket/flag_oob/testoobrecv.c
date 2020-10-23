/**
 * 测试服务端接收带外数据
 */
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("usage: %s port_number\n", basename(argv[0]));
        return 1;
    }
    int port = atoi(argv[1]);
    //绑定地址
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
    socklen_t client_length = sizeof(client);
    int connfd = accept(sock, (struct sockaddr *) &client, &client_length);
    if (connfd < 0) {
        printf("errno is: %d\n", errno);
    } else {
        //测试
        char buffer[BUFSIZ];
        memset(buffer, '\0', BUFSIZ);
        ret = recv(connfd, buffer, BUFSIZ - 1, 0);
        printf("got %d byte of normal data '%s'\n", ret, buffer);

        memset(buffer, '\0', BUFSIZ);
        ret = recv(connfd, buffer, BUFSIZ - 1, MSG_OOB);
        printf("got %d byte of normal data '%s'\n", ret, buffer);

        memset(buffer, '\0', BUFSIZ);
        ret = recv(connfd, buffer, BUFSIZ - 1, 0);
        printf("got %d byte of normal data '%s'\n", ret, buffer);

        close(connfd);
    }
}
