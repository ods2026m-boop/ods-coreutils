#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: mv src dst\n");
        return 0;
    }
    if (argc != 3) {
        fprintf(stderr, "Usage: mv src dst\n");
        return 1;
    }
    if (rename(argv[1], argv[2]) == 0) return 0;
    if (errno != EXDEV) {
        perror("mv");
        return 1;
    }
    struct stat st;
    if (stat(argv[1], &st) < 0) {
        perror(argv[1]);
        return 1;
    }
    if (S_ISDIR(st.st_mode)) {
        fprintf(stderr, "mv: cross-device directory move not implemented\n");
        return 1;
    }
    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 < 0) {
        perror(argv[1]);
        return 1;
    }
    int fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (fd2 < 0) {
        perror(argv[2]);
        close(fd1);
        return 1;
    }
    char buf[8192];
    ssize_t n;
    while ((n = read(fd1, buf, sizeof(buf))) > 0) {
        if (write(fd2, buf, n) != n) {
            perror(argv[2]);
            close(fd1);
            close(fd2);
            return 1;
        }
    }
    close(fd1);
    close(fd2);
    unlink(argv[1]);
    return 0;
}
