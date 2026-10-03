#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <sys/stat.h>

static int opt_name = 0;
static const char *pattern = NULL;
static int opt_type = 0;
static const char *type_str = NULL;

static int wildcard_match(const char *s, const char *p)
{
    if (*p == '\0') return *s == '\0';
    if (*p == '*') {
        while (*p == '*') p++;
        while (*s) {
            if (wildcard_match(s, p)) return 1;
            s++;
        }
        return wildcard_match(s, p);
    } else if (*p == '?') {
        if (*s == '\0') return 0;
        return wildcard_match(s + 1, p + 1);
    } else if (*p == *s) {
        return wildcard_match(s + 1, p + 1);
    }
    return 0;
}

static int callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    (void)sb;
    (void)ftwbuf;
    int match = 1;
    if (opt_name) {
        const char *base = strrchr(fpath, '/');
        base = base ? base + 1 : fpath;
        match = wildcard_match(base, pattern);
    }
    if (opt_type) {
        if (typeflag == FTW_F && strcmp(type_str, "f") != 0) match = 0;
        if (typeflag == FTW_D && strcmp(type_str, "d") != 0) match = 0;
    }
    if (match) puts(fpath);
    return 0;
}

int main(int argc, char *argv[])
{
    const char *dir = ".";
    int i = 1;
    if (i < argc && argv[i][0] != '-') {
        dir = argv[i++];
    }
    for (; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: find [dir] [-name pattern] [-type f|d]\n");
            return 0;
        } else if (strcmp(argv[i], "-name") == 0 && i + 1 < argc) {
            opt_name = 1;
            pattern = argv[++i];
        } else if (strcmp(argv[i], "-type") == 0 && i + 1 < argc) {
            opt_type = 1;
            type_str = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "find: unknown option: %s\n", argv[i]);
            return 1;
        }
    }
    nftw(dir, callback, 20, FTW_PHYS);
    return 0;
}
