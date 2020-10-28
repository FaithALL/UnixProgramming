/**
 * 创建echo服务器
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 0;
    }
    const int port = atoi(argv[1]);

    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(serverfd >= 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    //绑定socket地址
    int ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(serverfd, 5);
    assert(ret != -1);

    while (1) {
        struct sockaddr_in client;
        socklen_t clientlength = sizeof(client);
        int clientsock = accept(serverfd, (struct sockaddr *) &client, &clientlength);
        assert(clientsock >= 0);

        char buf[1024];
        int count;
        while ((count = read(clientsock, buf, 1024)) != 0) {
            write(STDOUT_FILENO, buf, count);
            write(clientsock, buf, count);
        }
        close(clientsock);
    }

    close(serverfd);
    return 0;
}
