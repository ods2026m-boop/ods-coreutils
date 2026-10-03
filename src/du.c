#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static int opt_h = 0;

static unsigned long long du(const char *path)
{
    struct stat st;
    if (lstat(path, &st) < 0) {
        perror(path);
        return 0;
    }
    if (!S_ISDIR(st.st_mode)) {
        return (unsigned long long)st.st_blocks * 512ULL;
    }
    unsigned long long total = (unsigned long long)st.st_blocks * 512ULL;
    char full[4096];
    DIR *d = opendir(path);
    if (!d) {
        perror(path);
        return total;
    }
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (ent->d_name[0] == '.' &&
            (ent->d_name[1] == '\0' ||
             (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
            continue;
        snprintf(full, sizeof(full), "%s/%s", path, ent->d_name);
        total += du(full);
    }
    closedir(d);
    return total;
}

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
            printf("Usage: du [-h] [path]\n");
            return 0;
        } else if (strcmp(argv[i], "-h") == 0) {
            opt_h = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "du: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    const char *path = argv[i];
    if (!path) path = ".";
    unsigned long long total = du(path);
    if (opt_h) human(total);
    else printf("%llu", total);
    printf(" %s\n", path);
    return 0;
}
