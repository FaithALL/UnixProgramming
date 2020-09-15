#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

char* env_init[] = {"USER=unknown", "PATH=/tmp", NULL};

int main() {
    pid_t pid;

    if ((pid = fork()) < 0)
        fprintf(stderr, "fork error\n");
    else if (pid == 0)
        if (execle("echoall", "echoall", "myarg1", "MY ARG2", (char*)0, env_init) < 0)
            fprintf(stderr, "execle error\n");
    
    if (waitpid(pid, NULL, 0) < 0)
        fprintf(stderr, "wait error\n");

    if ((pid = fork()) < 0)
        fprintf(stderr, "fork error\n");
    else if (pid == 0) {
        if (execlp("./echoall", "echoall", "only 1 arg", (char*) 0) < 0)
            fprintf(stderr, "execlp error\n");
    }
    return 0;
}
