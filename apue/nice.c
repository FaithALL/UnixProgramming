#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

unsigned long long count;
struct timeval end;

void checktime(char* str) {
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    if (tv.tv_sec >= end.tv_sec && tv.tv_usec >= end.tv_usec) {
        printf("%s count = %lld\n", str, count);
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    pid_t pid;
    char* s;
    int nzero, ret;
    int adj = 0;

    setbuf(stdout, NULL);
#if defined(NZERO)
    nzero = NZERO;
#elif defined(_SC_NZERO)
    nzero = sysconf(_SC_NZERO);
#else
#error NZERO undefined
#endif
    printf("NZERO = %d\n", nzero);
    if (argc == 2)
        adj = strtol(argv[1], NULL, 10);
    gettimeofday(&end, NULL);
    end.tv_sec += 10;

    if ((pid = fork()) < 0)
        fprintf(stderr, "fork failed\n");
    else if (pid == 0) {
        s = "child";
        printf("current nice valude in child is %d, agjusting by %d\n", nice(0) + nzero, adj);

        errno = 0;
        if ((ret = nice(adj)) == -1 && errno != 0)
            fprintf(stderr, "child set scheduling priority\n");
        printf("now child nice value is %d\n", ret + nzero);
    } else {
        s = "parent";
        printf("current nice value in parent is %d\n", nice(0) + nzero);
    }

    while (1) {
        if (++count == 0)
            fprintf(stderr, "%s counter wrap\n", s);
        checktime(s);
    }

    return 0;
}
