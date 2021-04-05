/**
 * POSIX 消息队列，写，需加-lrt链接选项，POSIX消息队列的使用更加类似对文件操作
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdlib.h>
#include <assert.h>

int main() {

    //打开或创建一个posix消息队列，可在/dev/mqueue看到该文件
    //名字必须以/开头
    mqd_t mqd = mq_open("/posixmq", O_CREAT | O_RDWR, 0666, NULL);

    char buf[] = "message queue test";

    //发送消息，最后一个参数表示优先级，0代表最低优先级
    mq_send(mqd, buf, sizeof(buf), 0);

    //关闭消息队列
    mq_close(mqd);

    return 0;
}
