#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static mode_t parse_octal(const char *s)
{
    char *end;
    long v = strtol(s, &end, 8);
    if (*end != '\0' || v < 0 || v > 07777) {
        fprintf(stderr, "chmod: invalid mode: '%s'\n", s);
        return (mode_t)-1;
    }
    return (mode_t)v;
}

static mode_t parse_symbolic(const char *s, mode_t cur)
{
    mode_t result = cur;
    const char *p = s;
    while (*p) {
        int who = 0;
        if (*p == 'a') {
            who = 7;
            p++;
        } else {
            while (*p == 'u' || *p == 'g' || *p == 'o') {
                if (*p == 'u') who |= 4;
                else if (*p == 'g') who |= 2;
                else if (*p == 'o') who |= 1;
                p++;
            }
        }
        if (who == 0) who = 7;
        char op = *p++;
        if (op != '+' && op != '-' && op != '=') {
            fprintf(stderr, "chmod: invalid mode\n");
            return (mode_t)-1;
        }
        char perm = *p++;
        if (!perm) {
            fprintf(stderr, "chmod: invalid mode\n");
            return (mode_t)-1;
        }
        mode_t bits = 0;
        if (perm == 'r') bits = 0444;
        else if (perm == 'w') bits = 0222;
        else if (perm == 'x') bits = 0111;
        else {
            fprintf(stderr, "chmod: invalid mode\n");
            return (mode_t)-1;
        }
        mode_t mask = 0;
        if (who & 4) mask |= 0700;
        if (who & 2) mask |= 0070;
        if (who & 1) mask |= 0007;
        mode_t affected = mask & bits;
        if (op == '=') result = (result & ~mask) | affected;
        else if (op == '+') result |= affected;
        else if (op == '-') result &= ~affected;
    }
    return result;
}

int main(int argc, char *argv[])
{
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: chmod mode file...\n");
        return 0;
    }
    if (argc < 3) {
        fprintf(stderr, "Usage: chmod mode file...\n");
        return 1;
    }
    mode_t mode = 0;
    if (argv[1][0] >= '0' && argv[1][0] <= '7') {
        mode = parse_octal(argv[1]);
        if (mode == (mode_t)-1) return 1;
    } else {
        for (int i = 2; i < argc; i++) {
            struct stat st;
            if (stat(argv[i], &st) < 0) {
                perror(argv[i]);
                return 1;
            }
            mode_t newmode = parse_symbolic(argv[1], st.st_mode);
            if (newmode == (mode_t)-1) return 1;
            if (chmod(argv[i], newmode) < 0) {
                perror(argv[i]);
                return 1;
            }
        }
        return 0;
    }
    for (int i = 2; i < argc; i++) {
        if (chmod(argv[i], mode) < 0) {
            perror(argv[i]);
            return 1;
        }
    }
    return 0;
}
