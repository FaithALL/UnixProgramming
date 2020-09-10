#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        return 0;
    }

    const char* ip = argv[1];
    const int  port = atoi(argv[2]);
    
    //创建ipv4 socket
    int clientfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    //创建server sock地址
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server.sin_addr);
    server.sin_port = htons(port);

    if (connect(clientfd, (struct sockaddr*)&server, sizeof(server)) != 0) {
        fprintf(stderr, "failed to connect server\n");
        return 0;
    }

    char buf[1024];
    while (fgets(buf, 1024, stdin) != NULL) {
        send(clientfd, buf, strlen(buf), 0);
        recv(clientfd, buf, strlen(buf), 1024);
        fputs(buf, stdout);
    }

    close(clientfd);
    return 0;
}
