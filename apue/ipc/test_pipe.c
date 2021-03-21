#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main() {
    int pipefd[2];
    pipe(pipefd);
    char buf;

    pid_t cpid = fork();
    if (cpid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (cpid == 0) {
        //子进程关闭写端
        close(pipefd[1]);

        while (read(pipefd[0], &buf, 1) > 0) 
            write(STDOUT_FILENO, &buf, 1);

        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    } else {
        //父进程关闭读端
        close(pipefd[0]);

        write(pipefd[1], "hello wrold\n", 12);

        close(pipefd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}
