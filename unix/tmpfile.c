#include <stdio.h>

int main() {
    char name[L_tmpnam], line[1024];
    FILE* fp;

    printf("%s\n", tmpnam(NULL));

    tmpnam(name);
    printf("%s\n", name);

    if ((fp = tmpfile()) == NULL)
        fprintf(stderr, "tmpfile error\n");

    fputs("one line output\n", fp);

    rewind(fp);

    if (fgets(line, sizeof(line), fp) == NULL)
        fprintf(stderr, "fgets error\n");

    fputs(line, stdout);
    return 0;
}
