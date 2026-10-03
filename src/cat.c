#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void cat(FILE *f)
{
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        fwrite(buf, 1, n, stdout);
    }
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: cat [file...]\n");
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "cat: unknown option: %s\n", argv[i]);
            return 1;
        }
    }
    if (argc == 1) {
        cat(stdin);
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == '\0') {
            cat(stdin);
        } else {
            FILE *f = fopen(argv[i], "rb");
            if (!f) {
                perror(argv[i]);
                return 1;
            }
            cat(f);
            fclose(f);
        }
    }
    return 0;
}
