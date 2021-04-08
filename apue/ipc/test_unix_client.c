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
    //使用Linux特有抽象路径
    strcpy(&address.sun_path[1], "unixsocket");
    connect(sockfd, (struct sockaddr*)&address, sizeof(address));

    write(sockfd, "hello server", 13);

    char buf[1024];
    read(sockfd, buf, 1024);
    printf("%s\n", buf);


    close(sockfd);
    return 0;
}
