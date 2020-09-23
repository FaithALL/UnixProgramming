#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void sig_quit(int);

int main(){
    sigset_t newmask, oldmask, pendmask;

    if (signal(SIGQUIT, sig_quit) == SIG_ERR) {
        fprintf(stderr, "can't catch SIGQUIT\n");
    }

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGQUIT);
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        fprintf(stderr, "SIG_BLOCK error\n");
    }

    sleep(5);

    if (sigpending(&pendmask) < 0) {
        fprintf(stderr, "sigpending error\n");
    }

    if (sigismember(&pendmask, SIGQUIT))
        fprintf(stderr, "SIGQUIT pending\n");

    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
        fprintf(stderr, "SIG_SETMASK error\n");
    printf("SIGQUIT unblocked\n");

    sleep(5);
    return 0;
}

static void sig_quit(int signo) {
    printf("caught SIGQUIT\n");

    if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
        fprintf(stderr, "can't reset SIGQUIT\n");
    }
}
