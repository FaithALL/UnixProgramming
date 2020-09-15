#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

void pr_exit(int status) {
    if (WIFEXITED(status)) 
        printf("normal termination, exit status = %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status),
#ifdef WCOREDUMP
                WCOREDUMP(status) ? " (core file generated)" : "");
#else 
                "");
#endif
    else if (WIFSTOPPED(status))
        printf("child stopped, signal number = %d\n", WSTOPSIG(status));
}

int system(const char* cmdstring) {
    pid_t pid;
    int status;

    if (cmdstring == NULL) return 1;

    if ((pid = fork()) < 0) status = -1;
    else if (pid == 0) {
        execl("/bin/sh", "sh", "-c", cmdstring, (char*)0);
        _exit(127);
    } else {
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1;
                break;
            }
        }
    }

    return status;
}

int main() {
    int status;
    if ((status = system("nosuchcommand")) < 0)
        fprintf(stderr, "system() error\n");

    pr_exit(status);

    if ((status = system("who; exit 44")) < 0)
        fprintf(stderr, "system() error\n");

    pr_exit(status);
    
    return 0;
}
