#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";

int main() {
    int fd;
    if ((fd = creat("file.hole", S_IRUSR | S_IWUSR)) < 0) {
        fprintf(stderr, "cannot creat file\n");
        return 0;
    }

    if (write(fd, buf1, 10) != 10) {
        fprintf(stderr, "buf1 write error\n");
        return 0;
    }

    if (lseek(fd, 16384, SEEK_SET) == -1) {
        fprintf(stderr, "lseek error\n");
        return 0;
    }

    if (write(fd, buf2, 10) != 10) {
        fprintf(stderr, "buf2 write error\n");
        return 0;
    }
    return 0;
}
