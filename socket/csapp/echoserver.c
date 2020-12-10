//csapp版echo服务端
#include "socket_package.c"
#include "rio.c"
#include <stdio.h>
#include <libgen.h>

void echo(int connfd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port\n", basename(argv[0]));
        return 1;
    }

    int listenfd = open_listenfd(argv[1], 5);
    int connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        getnameinfo((struct sockaddr *) &clientaddr,
                    clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        close(connfd);
    }
    return 0;
}

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

