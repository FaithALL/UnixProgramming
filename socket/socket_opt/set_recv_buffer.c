/**
 * 修改服务端接收缓冲区
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <assert.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc <= 2) {
        printf("usage: %s port_number recv_buffer_size\n", basename(argv[0]));
        return 1;
    }
    int port = atoi(argv[1]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    int recvbuf = atoi(argv[2]);
    int len = sizeof(recvbuf);

    //设置获取接收缓冲区
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof(recvbuf));
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t *) &len);
    printf("the tcp receive buffer size after setting is %d\n", recvbuf);

    int ret = bind(sock, (struct sockaddr *) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr *) &client, &client_addrlength);

    if (connfd < 0) printf("errno is : %d\n", errno);
    else {
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        while (recv(connfd, buffer, BUFFER_SIZE - 1, 0) > 0);
        close(connfd);
    }
    return 0;
}