#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <time.h>

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: touch file...\n");
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "touch: unknown option: %s\n", argv[i]);
            return 1;
        }
    }
    for (int i = 1; i < argc; i++) {
        struct stat st;
        if (stat(argv[i], &st) == 0) {
            struct utimbuf ub;
            ub.actime = time(NULL);
            ub.modtime = time(NULL);
            utime(argv[i], &ub);
        } else {
            int fd = open(argv[i], O_WRONLY | O_CREAT | O_NOCTTY, 0644);
            if (fd < 0) {
                perror(argv[i]);
                return 1;
            }
            close(fd);
        }
    }
    return 0;
}
