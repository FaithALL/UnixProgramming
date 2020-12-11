/*
 * 基于进程的并发echo服务器
 */

#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "socket_package.c"
#include "rio.c"

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

void sigchld_handler(int sig) {
    while (waitpid(-1, 0, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port\n", argv[0]);
        return 1;
    }

    signal(SIGCHLD, sigchld_handler);
    int listenfd = open_listenfd(argv[1], 5);
    socklen_t clientlen;
    int connfd;
    struct sockaddr_storage clientaddr;

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        if (fork() == 0) {
            close(listenfd);
            echo(connfd);
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
}
