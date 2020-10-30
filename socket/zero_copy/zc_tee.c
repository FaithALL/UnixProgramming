/**
 * 测试tee函数，完成tee程序(同时输出数据到终端和文件)
 * ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags);
 * fd_in fd_out必须是管道文件
 */
#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <file>", basename(argv[0]));
        return 1;
    }

    int filefd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
    assert(filefd > 0);

    int pipefd_stdout[2];
    int ret = pipe(pipefd_stdout);
    assert(ret != -1);

    int pipefd_file[2];
    ret = pipe(pipefd_file);
    assert(ret != -1);
    //从标准输入输入到管道pipefd_stdout
    ret = splice(STDIN_FILENO, NULL, pipefd_stdout[1], NULL, 32768, SPLICE_F_MOVE | SPLICE_F_MORE);
    assert(ret != -1);
    //从pipefd_stdout输出到pipefd_file
    ret = tee(pipefd_stdout[0], pipefd_file[1], 32768, SPLICE_F_NONBLOCK);
    assert(ret != -1);

    //pipefd_file输出到文件
    ret = splice(pipefd_file[0], NULL, filefd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
    assert(ret != -1);

    //pipefd_stdout输出到标准输出
    ret = splice(pipefd_stdout[0], NULL, STDOUT_FILENO, NULL, 32768, SPLICE_F_MOVE | SPLICE_F_MORE);

    assert(ret != -1);
    close(filefd);
    close(pipefd_stdout[0]);
    close(pipefd_stdout[1]);
    close(pipefd_file[0]);
    close(pipefd_file[1]);
}