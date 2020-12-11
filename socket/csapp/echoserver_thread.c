/**
 * 基于多线程的并发echo服务器
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "rio.c"
#include "socket_package.c"

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

void *thread(void *vargp) {
    int connfd = *((int *) vargp);
    pthread_detach(pthread_self());
    free(vargp);
    echo(connfd);
    close(connfd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port\n", argv[0]);
        return 1;
    }
    int listenfd = open_listenfd(argv[1], 5);

    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }
}

