#include <unistd.h>
#include <stdio.h>

#define BUFFSISE 4096

int main() {
    int n;
    char buf[BUFFSISE];

    while ((n = read(STDIN_FILENO, buf, BUFFSISE)) > 0) 
        if (write(STDOUT_FILENO, buf, n) != n) {
            fprintf(stderr, "write error\n");
            return 0;
        }

    if (n < 0) fprintf(stderr, "read error\n");
    return 0;
}
