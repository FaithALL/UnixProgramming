#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int globvar = 6;
char buf[] = "a write to stdout\n";

int main() {
    int var;
    pid_t pid;

    var = 88;
    if (write(STDOUT_FILENO, buf, sizeof(buf) - 1) != sizeof(buf) - 1)
        fprintf(stderr, "write error\n");
    printf("before fork\n");

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
    } else if (pid == 0) {
        globvar++;
        var++;
    } else {
        sleep(2);
    }

    printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar, var);

    return 0;
}

