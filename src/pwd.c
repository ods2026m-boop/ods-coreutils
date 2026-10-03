#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: pwd\n");
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "pwd: unknown option: %s\n", argv[i]);
            return 1;
        }
    }
    char buf[4096];
    if (getcwd(buf, sizeof(buf))) {
        puts(buf);
        return 0;
    }
    return 1;
}
