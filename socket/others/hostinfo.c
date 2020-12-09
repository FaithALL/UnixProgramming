/**
 * 通过域名获取ip地址
 */
#include <stdio.h>
#include <string.h>
#include <netdb.h>

#define MAXLINE 128

int main(int argc, char *argv[]) {
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        return 1;
    }

    //过滤条件
    memset(&hints, 0, sizeof(hints));
//    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    //查询
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        return 1;
    }

    //显示地址而非域名
    flags = NI_NUMERICHOST;
    for (p = listp; p; p = p->ai_next) {
        //转换为点分十进制
        getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }
    freeaddrinfo(listp);
    return 0;
}