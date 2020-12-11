/**
 * 基于select IO复用的并发echo服务器
 */
#include "rio.c"
#include "socket_package.c"
#include <stdio.h>
#include <stdlib.h>

void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    rio_readinitb(&rio, connfd);
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int) n);
        rio_writen(connfd, buf, n);
    }
}

void commmand() {
    char buf[MAXLINE];
    if (!fgets(buf, MAXLINE, stdin)) exit(0);
    printf("%s", buf);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port\n", argv[0]);
        return 1;
    }

    int listenfd = open_listenfd(argv[1], 5);
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    fd_set read_set, ready_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(listenfd, &read_set);
    int connfd;

    while (1) {
        ready_set = read_set;
        select(listenfd + 1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &ready_set)) commmand();
        if (FD_ISSET(listenfd, &ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
            echo(connfd);
            close(connfd);
        }
    }
}
