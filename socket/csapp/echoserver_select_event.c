/**
 * 基于select的事件驱动并发echo服务器
 */

#include "rio.c"
#include "socket_package.c"
#include <stdio.h>

typedef struct {
    int maxfd;          //read_set里最大的文件描述符
    fd_set read_set;
    fd_set ready_set;
    int nready;         //就绪的文件描述符数
    int maxi;
    int clientfd[FD_SETSIZE]; //活动文件描述符的集合
    rio_t clientrio[FD_SETSIZE];
} pool;

//服务器收到的字节总数
int byte_cnt = 0;

//初始化客户端池
void init_pool(int listenfd, pool *p) {
    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; ++i) p->clientfd[i] = -1;
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

//添加一个新的客户连接端到活动客户端池中
void add_client(int connfd, pool *p) {
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; ++i) {
        //空闲
        if (p->clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            rio_readinitb(&p->clientrio[i], connfd);
            FD_SET(connfd, &p->read_set);
            if (connfd > p->maxfd) p->maxfd = connfd;
            if (i > p->maxi) p->maxi = i;
            break;
        }
    }
    if (i == FD_SETSIZE) fprintf(stderr, "add_client error: Too many clients");
}

//回送已读取文本行回客户端
void check_clients(pool *p) {
    int i, connfd, n;
    char buf[MAXLINE];
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); ++i) {
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
            p->nready--;
            if ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
                byte_cnt += n;
                printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
                rio_writen(connfd, buf, n);
            } else {
                //EOF
                close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port\n", argv[0]);
        return 1;
    }

    int listenfd = open_listenfd(argv[1], 5);
    int connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    static pool pool;
    init_pool(listenfd, &pool);

    while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
            add_client(connfd, &pool);
        }
        check_clients(&pool);
    }
}