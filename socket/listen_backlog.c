/**
 * 测试backlog参数对listen调用的影响
 * linux下测试完整连接的个数上限是backlog+1
 */
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <assert.h>
#include <strings.h>
#include <netinet/in.h>
#include <unistd.h>

static bool stop = false;

//信号处理,用于终止循环
static void handle_term(int sig) {
    printf("ending\n");
    stop = true;
}

int main(int argc, char *argv[]) {
    //SIGTERM可以通过kill -15产生
    signal(SIGTERM, handle_term);

    if (argc <= 2) {
        printf("usage: %s port_number backlog\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
    int backlog = atoi(argv[2]);

    //使用tcp socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    //创建ipv4 socket地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    //绑定地址
    int ret = bind(sock, (struct sockaddr *) &address, sizeof(address));
    assert(ret != -1);
    //监听socket
    ret = listen(sock, backlog);
    assert(ret != -1);
    while (!stop) sleep(1);

    close(sock);
    return 0;
}

