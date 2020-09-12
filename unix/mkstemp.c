#include <asm-generic/errno-base.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

void make_temp(char* template) {
    int fd;
    struct stat sbuf;

    if ((fd = mkstemp(template)) < 0)
        fprintf(stderr, "can't create temp file");

    printf("temp name = %s\n", template);
    close(fd);

    if (stat(template, &sbuf) < 0) {
        if (errno == ENOENT)
            printf("file doesn't exist\n");
        else
            fprintf(stderr, "stat failed\n");
    } else {
        printf("file exist\n");
        unlink(template);
    }
}

int main() {
    char good_template[] = "/tmp/dirXXXXXX";
    char* bad_template = "/tmp/dirXXXXXX";

    printf("trying to create first temp file ...\n");
    make_temp(good_template);
    printf("trying to create second temp file ...\n");
    make_temp(bad_template);
    return 0;
}
