#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s directory_name\n", basename(argv[0]));
        return 1;
    }
    DIR *streamp;
    struct dirent *dep;
    streamp = opendir(argv[1]);
    errno = 0;
    while ((dep = readdir(streamp)) != NULL)
        printf("Found file: %s\n", dep->d_name);
    if (errno != 0) fprintf(stderr, "readdir error\n");
    closedir(streamp);
    return 0;
}

