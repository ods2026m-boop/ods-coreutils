#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

static int opt_f = 0;
static int opt_r = 0;

static int rm_recursive(const char *path)
{
    struct stat st;
    if (lstat(path, &st) < 0) {
        if (!opt_f) perror(path);
        return 0;
    }
    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(path);
        if (!d) {
            if (!opt_f) perror(path);
            return 0;
        }
        struct dirent *ent;
        char full[4096];
        int ret = 0;
        while ((ent = readdir(d))) {
            if (ent->d_name[0] == '.' &&
                (ent->d_name[1] == '\0' ||
                 (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
                continue;
            snprintf(full, sizeof(full), "%s/%s", path, ent->d_name);
            if (rm_recursive(full) < 0) ret = 1;
        }
        closedir(d);
        if (rmdir(path) < 0) {
            if (!opt_f) {
                perror(path);
                ret = 1;
            }
        }
        return ret;
    }
    if (unlink(path) < 0) {
        if (!opt_f) {
            perror(path);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: rm [-f] [-r] file...\n");
            return 0;
        } else if (strcmp(argv[i], "-f") == 0) opt_f = 1;
        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0) opt_r = 1;
        else if (argv[i][0] == '-') {
            fprintf(stderr, "rm: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    if (i >= argc) {
        fprintf(stderr, "Usage: rm [-f] [-r] file...\n");
        return 1;
    }
    int ret = 0;
    for (; i < argc; i++) {
        struct stat st;
        if (lstat(argv[i], &st) == 0 && S_ISDIR(st.st_mode)) {
            if (!opt_r) {
                fprintf(stderr, "rm: cannot remove '%s': is a directory\n", argv[i]);
                ret = 1;
                continue;
            }
            if (rm_recursive(argv[i]) < 0) ret = 1;
        } else {
            if (unlink(argv[i]) < 0) {
                if (!opt_f) {
                    perror(argv[i]);
                    ret = 1;
                }
            }
        }
    }
    return ret;
}
