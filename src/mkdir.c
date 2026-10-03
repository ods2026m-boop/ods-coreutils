#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

static int mkdir_p(const char *path, mode_t mode)
{
    char tmp[4096];
    char *p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, mode) < 0 && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, mode) < 0 && errno != EEXIST) return -1;
    return 0;
}

int main(int argc, char *argv[])
{
    int pflag = 0;
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: mkdir [-p] dir...\n");
            return 0;
        } else if (strcmp(argv[i], "-p") == 0) pflag = 1;
        else if (argv[i][0] == '-') {
            fprintf(stderr, "mkdir: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    if (i >= argc) {
        fprintf(stderr, "mkdir: missing operand\n");
        return 1;
    }
    for (; i < argc; i++) {
        if (pflag) {
            if (mkdir_p(argv[i], 0755) < 0) {
                perror(argv[i]);
                return 1;
            }
        } else {
            if (mkdir(argv[i], 0755) < 0) {
                perror(argv[i]);
                return 1;
            }
        }
    }
    return 0;
}
