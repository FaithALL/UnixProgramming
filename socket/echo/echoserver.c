#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 0;
    }

    const int port = atoi(argv[1]);
    const char* ip = "127.0.0.1";
    
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(serverfd >= 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int ret = bind(serverfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    
    ret = listen(serverfd, 5);
    assert(ret != -1);

    while (1) {
        struct sockaddr_in client;
        socklen_t clientlength = sizeof(client);
        int clientsock = accept(serverfd, (struct sockaddr*)&client, &clientlength);
        assert(clientsock >= 0);

        char remote[INET_ADDRSTRLEN];
        printf("connected with ip: %s and port: %d\n", 
                inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN), ntohs(client.sin_port));

        char buf[1024];
        while (recv(clientsock, buf, 1024, 0) != 0) {
            printf("-->%s", buf);
            if (strlen(buf) >= 3) {
                buf[0] = '<';
                buf[2] = buf[1] = '-';
            }
            send(clientsock, buf, strlen(buf), 0);
        }
        close(serverfd);
    }
    return 0;
}
