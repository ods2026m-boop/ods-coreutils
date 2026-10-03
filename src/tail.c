#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    long lines = 10;
    int argi = 1;
    for (; argi < argc; argi++) {
        if (strcmp(argv[argi], "-h") == 0 || strcmp(argv[argi], "--help") == 0) {
            printf("Usage: tail [-n lines] file\n");
            return 0;
        } else if (strncmp(argv[argi], "-n", 2) == 0) {
            if (argv[argi][2] == '\0') {
            if (argi + 1 >= argc) {
                fprintf(stderr, "tail: missing argument for -n\n");
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
            fprintf(stderr, "tail: unknown option: %s\n", argv[argi]);
            return 1;
        } else break;
    }
    if (argi >= argc) {
        fprintf(stderr, "tail: missing file\n");
        return 1;
    }
    FILE *f = fopen(argv[argi], "rb");
    if (!f) {
        perror(argv[argi]);
        return 1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(f);
        return 1;
    }
    long size = ftell(f);
    if (size < 0) {
        perror("ftell");
        fclose(f);
        return 1;
    }
    char *buf = malloc(size > 0 ? (size_t)size : 1);
    if (!buf) {
        perror("malloc");
        fclose(f);
        return 1;
    }
    fseek(f, 0, SEEK_SET);
    size_t n = fread(buf, 1, (size_t)size, f);
    long count = 0;
    for (long i = (long)n - 1; i >= 0; i--) {
        if (buf[i] == '\n') {
            count++;
            if (count > lines) {
                fwrite(buf + (size_t)(i + 1), 1, n - (size_t)(i + 1), stdout);
                free(buf);
                fclose(f);
                return 0;
            }
        }
    }
    fwrite(buf, 1, n, stdout);
    free(buf);
    fclose(f);
    return 0;
}
