/**
 * System V消息队列，写
 */

#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

//消息队列自定义消息格式，但类型要是如下形式(long + 字符数组)
struct mymesg {
    long mtype;         //指定类型，也可用作优先级
    char mtext[512];    //消息内容
};

int main() {
    //由路径名和项目id生成一个key
    key_t key = ftok("./test_mqw.c", 0);

    //打开或创建一个指定key的，消息队列
    int msg_id = msgget(key, IPC_CREAT | 0666);

    struct mymesg msg;
    msg.mtype = 1;
    strcpy(msg.mtext, "message queue test");

    //发送消息，第3个参数指定消息大小(不含类型)
    msgsnd(msg_id, &msg, sizeof(msg.mtext), 0);

    return 0;
}
