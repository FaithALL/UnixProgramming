#include <sys/stat.h>
#include <stdio.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s file_name\n", basename(argv[0]));
        return 1;
    }
    struct stat st;
    char *type, *readok;
    stat(argv[1], &st);
    if (S_ISREG(st.st_mode)) type = "regular";
    else if (S_ISDIR(st.st_mode)) type = "directory";
    else type = "other";

    readok = (st.st_mode & S_IRUSR) ? "yes" : "no";
    printf("type: %s, read: %s\n", type, readok);
    return 0;
}

