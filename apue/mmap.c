#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define COPYINCR (1024 * 1024 * 1024)
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main(int argc, char* argv[]) {
    int         fdin, fdout;
    void        *src, *dst;
    size_t      copysz;
    struct stat sbuf;
    off_t       fsz = 0;

    if (argc != 3)
        fprintf(stderr, "usage: %s <fromfile> <tofile>\n", argv[0]);
    
    if ((fdin = open(argv[1], O_RDONLY)) < 0)
        fprintf(stderr, "can't open %s for reading\n", argv[1]);

    if ((fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) < 0)
        fprintf(stderr, "can't creat %s for writing\n", argv[2]);

    if (fstat(fdin, &sbuf) < 0)
        fprintf(stderr, "fsat error\n");

    if (ftruncate(fdout, sbuf.st_size) < 0)
        fprintf(stderr, "ftruncate error\n");

    while (fsz < sbuf.st_size) {
        if ((sbuf.st_size - fsz) > COPYINCR)
            copysz = COPYINCR;
        else 
            copysz = sbuf.st_size - fsz;

        if ((src = mmap(0, copysz, PROT_READ, MAP_SHARED, fdin, fsz)) == MAP_FAILED)
            fprintf(stderr, "mmap error for input\n");

        if ((dst = mmap(0, copysz, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, fsz)) == MAP_FAILED)
            fprintf(stderr, "mmap error for output\n");

        memcpy(dst, src, copysz);
        munmap(src, copysz);
        munmap(dst, copysz);
        fsz += copysz;
    }
    return 0;
}
