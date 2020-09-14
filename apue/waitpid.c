#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid;
    if ((pid = fork()) < 0)
        fprintf(stderr, "fork error\n");
    else if (pid == 0) {
        if ((pid = fork()) < 0)
            fprintf(stderr, "fork error\n");
        else if (pid > 0)
            exit(0);
        sleep(2);
        printf("second child, parent pid = %ld\n", (long)getppid());
        exit(0);
    }

    printf("pid = %ld\n", (long)pid);

    if (waitpid(pid, NULL, 0) != pid)
        fprintf(stderr, "waitpid error\n");

    return 0;
}
