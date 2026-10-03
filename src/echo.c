#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int newline = 1;
    int i = 1;
    for (; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: echo [-n] [arg...]\n");
            return 0;
        }
        if (strcmp(argv[i], "-n") == 0) {
            newline = 0;
            continue;
        }
        if (argv[i][0] == '-') {
            fprintf(stderr, "echo: unknown option: %s\n", argv[i]);
            return 1;
        }
        break;
    }
    for (; i < argc; i++) {
        fputs(argv[i], stdout);
        if (i < argc - 1) putchar(' ');
    }
    if (newline) putchar('\n');
    return 0;
}
