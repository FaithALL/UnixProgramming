//csapp版echo的客户端
#include "socket_package.c"
#include "rio.c"
#include <stdio.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s host port\n", basename(argv[0]));
        return 1;
    }

    int clientfd = open_clientfd(argv[1], argv[2]);
    rio_t rio;
    rio_readinitb(&rio, clientfd);
    char buf[MAXLINE];

    while (fgets(buf, MAXLINE, stdin) != NULL) {
        rio_writen(clientfd, buf, strlen(buf));
        rio_readlineb(&rio, buf, MAXLINE);
        fputs(buf, stdout);
    }
    close(clientfd);
    return 0;
}

