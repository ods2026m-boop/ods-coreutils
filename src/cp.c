#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

static int opt_r = 0;

static int copy_file(const char *src, const char *dst)
{
    int fd1 = open(src, O_RDONLY);
    if (fd1 < 0) {
        perror(src);
        return 1;
    }
    struct stat st;
    if (stat(src, &st) < 0) {
        perror(src);
        close(fd1);
        return 1;
    }
    int fd2 = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (fd2 < 0) {
        perror(dst);
        close(fd1);
        return 1;
    }
    char buf[8192];
    ssize_t n;
    while ((n = read(fd1, buf, sizeof(buf))) > 0) {
        if (write(fd2, buf, n) != n) {
            perror(dst);
            close(fd1);
            close(fd2);
            return 1;
        }
    }
    close(fd1);
    close(fd2);
    return 0;
}

static int copy_dir(const char *src, const char *dst)
{
    if (mkdir(dst, 0755) < 0 && errno != EEXIST) {
        perror(dst);
        return 1;
    }
    DIR *d = opendir(src);
    if (!d) {
        perror(src);
        return 1;
    }
    struct dirent *ent;
    char s[4096], d2[4096];
    int ret = 0;
    while ((ent = readdir(d))) {
        if (ent->d_name[0] == '.' &&
            (ent->d_name[1] == '\0' ||
             (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
            continue;
        snprintf(s, sizeof(s), "%s/%s", src, ent->d_name);
        snprintf(d2, sizeof(d2), "%s/%s", dst, ent->d_name);
        struct stat st;
        if (stat(s, &st) < 0) {
            perror(s);
            ret = 1;
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            if (copy_dir(s, d2) < 0) ret = 1;
        } else {
            if (copy_file(s, d2) < 0) ret = 1;
        }
    }
    closedir(d);
    return ret;
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: cp [-r] src dst\n");
            return 0;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0) opt_r = 1;
        else if (argv[i][0] == '-') {
            fprintf(stderr, "cp: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    if (i + 2 != argc) {
        fprintf(stderr, "Usage: cp [-r] src dst\n");
        return 1;
    }
    const char *src = argv[i];
    const char *dst = argv[i + 1];
    struct stat st;
    if (stat(src, &st) < 0) {
        perror(src);
        return 1;
    }
    if (S_ISDIR(st.st_mode)) {
        if (!opt_r) {
            fprintf(stderr, "cp: -r required for directories\n");
            return 1;
        }
        return copy_dir(src, dst);
    }
    return copy_file(src, dst);
}
