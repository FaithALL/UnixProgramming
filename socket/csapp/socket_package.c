//封装监听socket和连接socket
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <memory.h>

#define MAXLINE 1024

//建立到hostname:port的连接，返回可用于读写的文件描述符，失败返回-1
int open_clientfd(char *hostname, char *port) {
    struct addrinfo hints, *listp, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    //接受数字类型的端口
    hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
    if (getaddrinfo(hostname, port, &hints, &listp) != 0) return -1;

    int clientfd;
    for (p = listp; p; p = p->ai_next) {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue;
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) break;
        close(clientfd);
    }
    freeaddrinfo(listp);
    if (!p) return -1;
    else return clientfd;
}

//监听port端口，backlog为最大监听数，成功收到返回可accept的文件描述符，失败返回-1
int open_listenfd(char *port, int backlog) {
    struct addrinfo hints, *listp, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    //接受所有到本机port端口的连接
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    if (getaddrinfo(NULL, port, &hints, &listp) != 0) return -1;

    int listenfd, optval = 1;
    for (p = listp; p; p = p->ai_next) {
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(listenfd);
    }

    freeaddrinfo(listp);
    if (!p) return -1;
    if (listen(listenfd, backlog) < 0) {
        close(listenfd);
        return -1;
    }
    return listenfd;
}