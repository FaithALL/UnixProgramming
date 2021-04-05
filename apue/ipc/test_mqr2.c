/**
 * POSIX 消息队列，读，需加-lrt链接选项，POSIX消息队列的使用更加类似对文件操作
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>


int main() {

    //打开或创建一个posix消息队列，可在/dev/mqueue看到该文件
    //名字必须以/开头
    mqd_t mqd = mq_open("/posixmq", O_CREAT | O_RDWR, 0666, NULL);

    char buf[10240];

    //接收消息，会删除一条优先级最高，存在时间最长的消息
    //第三个参数，不论消息实际长度，它的值应该大于队列的mq_msgsize属性
    //最后一个参数可以获取优先级信息，NULL代表忽略优先级
    mq_receive(mqd, buf, sizeof(buf), NULL);

    printf("%s\n", buf);

    //关闭消息队列
    mq_close(mqd);

    //删除消息队列
    mq_unlink("/posixmq");

    return 0;
}
