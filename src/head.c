#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    long lines = 10;
    int argi = 1;
    for (; argi < argc; argi++) {
        if (strcmp(argv[argi], "-h") == 0 || strcmp(argv[argi], "--help") == 0) {
            printf("Usage: head [-n lines] file\n");
            return 0;
        } else if (strncmp(argv[argi], "-n", 2) == 0) {
            if (argv[argi][2] == '\0') {
            if (argi + 1 >= argc) {
                fprintf(stderr, "head: missing argument for -n\n");
                return 1;
            }
            lines = atol(argv[argi + 1]);
            argi++;
            continue;
            } else {
                lines = atol(argv[argi] + 2);
                argi++;
            }
        } else if (argv[argi][0] == '-') {
            fprintf(stderr, "head: unknown option: %s\n", argv[argi]);
            return 1;
        } else break;
    }
    if (argi >= argc) {
        fprintf(stderr, "head: missing file\n");
        return 1;
    }
    FILE *f = fopen(argv[argi], "r");
    if (!f) {
        perror(argv[argi]);
        return 1;
    }
    char buf[4096];
    while (lines > 0 && fgets(buf, sizeof(buf), f)) {
        fputs(buf, stdout);
        lines--;
    }
    fclose(f);
    return 0;
}
