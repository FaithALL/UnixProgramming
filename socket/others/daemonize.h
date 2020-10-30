/**
 * 编写守护进程，Linux下有库函数daemon
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

bool daemonize() {
    //1 umask 不使用继承的屏蔽字
    umask(0);

    //2 父进程 fork exit
    int pid = fork();
    if (pid < 0) return false;
    else if (pid > 0) exit(0);

    //3 创建新会话
    if (setsid() < 0) return false;

    //4 更改根目录
    if (chdir("/") < 0) return false;

    //5 关闭不需要的文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    //6 重定向 0 1 2文件描述符到/dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    return true;
}

