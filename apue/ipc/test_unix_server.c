/**
 * unix域套接字，参见https://blog.csdn.net/Roland_Sun/article/details/50266565
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main() {

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    //使用Linux特有抽象路径，当结束时自动删除
    strcpy(&address.sun_path[1], "unixsocket");

    bind(sockfd, (struct sockaddr*)&address, sizeof(address));
    
    listen(sockfd, 5);

    struct sockaddr_un clientAddress;
    socklen_t clientAddressLength;
    int clientFd = accept(sockfd, (struct sockaddr*)&clientAddress, &clientAddressLength);

    char buf[1024];
    read(clientFd, buf, 1024);
    write(clientFd, "i receive", 10);

    printf("%s\n", buf);

    close(clientFd);

    close(sockfd);

    return 0;
}
