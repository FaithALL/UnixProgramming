#include <stdlib.h>
#include <stdio.h>

static void my_exit1(void);
static void my_exit2(void);

int main() {
    if (atexit(my_exit2) != 0)
        fprintf(stderr, "can't register my_exit2\n");

    if (atexit(my_exit1) != 0) 
        fprintf(stderr, "can't register my_exit1\n");
    if (atexit(my_exit1) != 0) 
        fprintf(stderr, "can't register my_exit1\n");

    printf("main is done\n");

    return 0;
}

static void my_exit1(void) {
    printf("frist exit handler\n");
}

static void my_exit2(void) {
    printf("second exit handler\n");
}
