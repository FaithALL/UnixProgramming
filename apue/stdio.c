#include <stdio.h>

int main() {
//    int c;
//    while ((c = getc(stdin)) != EOF)
//        if (putc(c, stdout) == EOF) {
//            printf("output error\n");
//        }
//
//    if (ferror(stdin))
//        printf("input error\n");
    char buf[1024];
    while (fgets(buf, 1024, stdin) != NULL)
        if (fputs(buf, stdout) == EOF)
            printf("output error\n");

    if (ferror(stdin)) printf("input error\n");
    return 0;
}
