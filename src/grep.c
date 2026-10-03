#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <dirent.h>
#include <sys/stat.h>

static int opt_i = 0;
static int opt_n = 0;
static int opt_v = 0;
static int opt_r = 0;

static int grep_file(const char *path, const char *pattern)
{
    regex_t re;
    int flags = REG_EXTENDED | (opt_i ? REG_ICASE : 0);
    if (regcomp(&re, pattern, flags) != 0) {
        fprintf(stderr, "grep: invalid regex\n");
        return 2;
    }
    FILE *f;
    if (strcmp(path, "-") == 0) f = stdin;
    else {
        f = fopen(path, "r");
        if (!f) {
            perror(path);
            regfree(&re);
            return 2;
        }
    }
    char buf[4096];
    int line = 0;
    int found = 0;
    while (fgets(buf, sizeof(buf), f)) {
        line++;
        int match = regexec(&re, buf, 0, NULL, 0) == 0;
        if (opt_v) match = !match;
        if (match) {
            found = 1;
            if (opt_n && f != stdin) printf("%s:%d:", path, line);
            fputs(buf, stdout);
        }
    }
    if (f != stdin) fclose(f);
    regfree(&re);
    return found;
}

static int grep_recursive(const char *dir, const char *pattern)
{
    DIR *d = opendir(dir);
    if (!d) {
        perror(dir);
        return 2;
    }
    struct dirent *ent;
    char full[4096];
    int found = 0;
    while ((ent = readdir(d))) {
        if (ent->d_name[0] == '.' &&
            (ent->d_name[1] == '\0' ||
             (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
            continue;
        snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
        struct stat st;
        if (stat(full, &st) < 0) {
            perror(full);
            return 2;
        }
        int r;
        if (S_ISDIR(st.st_mode)) {
            if (opt_r) r = grep_recursive(full, pattern);
            else continue;
        } else {
            r = grep_file(full, pattern);
        }
        if (r == 2) return 2;
        if (r) found = 1;
    }
    closedir(d);
    return found;
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: grep [-invr] pattern [file...]\n");
            return 0;
        } else if (strcmp(argv[i], "-i") == 0) opt_i = 1;
        else if (strcmp(argv[i], "-n") == 0) opt_n = 1;
        else if (strcmp(argv[i], "-v") == 0) opt_v = 1;
        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0) opt_r = 1;
        else if (argv[i][0] == '-') {
            fprintf(stderr, "grep: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    if (i >= argc) {
        fprintf(stderr, "Usage: grep [-invr] pattern [file...]\n");
        return 1;
    }
    const char *pattern = argv[i++];
    if (i >= argc) {
        int r = grep_file("-", pattern);
        if (r == 2) return 2;
        return r ? 0 : 1;
    }
    int found = 0;
    int error = 0;
    for (; i < argc; i++) {
        struct stat st;
        if (stat(argv[i], &st) < 0) {
            perror(argv[i]);
            error = 1;
            continue;
        }
        int r;
        if (S_ISDIR(st.st_mode)) {
            if (!opt_r) {
                fprintf(stderr, "grep: %s is a directory\n", argv[i]);
                error = 1;
                continue;
            }
            r = grep_recursive(argv[i], pattern);
        } else {
            r = grep_file(argv[i], pattern);
        }
        if (r == 2) error = 1;
        else if (r) found = 1;
    }
    if (error) return 2;
    return found ? 0 : 1;
}
