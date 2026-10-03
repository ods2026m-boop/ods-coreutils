#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

static int opt_h = 0;

static void human(unsigned long long bytes)
{
    if (bytes < 1024) printf("%lluB", bytes);
    else if (bytes < 1024 * 1024) printf("%lluK", bytes / 1024);
    else if (bytes < 1024 * 1024 * 1024) printf("%lluM", bytes / (1024 * 1024));
    else printf("%lluG", bytes / (1024 * 1024 * 1024));
}

int main(int argc, char *argv[])
{
    int i = 1;
    for (; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: df [-h] [path]\n");
            return 0;
        } else if (strcmp(argv[i], "-h") == 0) {
            opt_h = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "df: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    const char *path = argv[i];
    if (!path) path = "/";
    struct statvfs sv;
    if (statvfs(path, &sv) < 0) {
        perror(path);
        return 1;
    }
    unsigned long long total = sv.f_blocks * sv.f_frsize;
    unsigned long long avail = sv.f_bfree * sv.f_frsize;
    unsigned long long used = total - avail;
    printf("Filesystem Size Used Avail Use%% Mounted on\n");
    if (opt_h) {
        printf("%-10s ", path);
        human(total); putchar(' ');
        human(used); putchar(' ');
        human(avail); putchar(' ');
        printf("100%% %s\n", path);
    } else {
        printf("%-10s %llu %llu %llu 100%% %s\n", path, total, used, avail, path);
    }
    return 0;
}
