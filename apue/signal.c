#include <signal.h>
#include <unistd.h>
#include <stdio.h>


static void sig_usr(int);

int main() {
    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
        fprintf(stderr, "can't catch SIGUSR1\n");
    if (signal(SIGUSR2, sig_usr) == SIG_ERR)
        fprintf(stderr, "can't catch SIGUSR2\n");

    while (1) pause();

    return 0;
}

static void sig_usr(int signo) {
    if (signo == SIGUSR1)
        printf("received SIGUSR1\n");
    else if (signo == SIGUSR2)
        printf("received SIGUSR2\n");
    else 
        fprintf(stderr, "received signal %d\n", signo);
}
