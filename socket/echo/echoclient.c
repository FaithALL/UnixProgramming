#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        return 0;
    }
    const char *ip = argv[1];
    const int port = atoi(argv[2]);

    //创建ipv4 socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);
    //创建server sock地址
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server.sin_addr);
    server.sin_port = htons(port);

    if (connect(clientfd, (struct sockaddr *) &server, sizeof(server)) != 0) {
        fprintf(stderr, "failed to connect server\n");
        return 0;
    }

    char buf[1024];
    int count;
    //从终端读取
    while ((count = read(STDIN_FILENO, buf, 1024)) != 0) {
        write(clientfd, buf, count);
        recv(clientfd, buf, count, MSG_WAITALL);
        write(STDOUT_FILENO, buf, count);
    }

    close(clientfd);
    return 0;
}
