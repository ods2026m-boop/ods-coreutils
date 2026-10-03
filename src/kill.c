#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <strings.h>

int main(int argc, char *argv[])
{
    int sig = SIGTERM;
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: kill [-signal] pid...\n");
            return 0;
        } else if (argv[i][0] == '-') {
            const char *s = argv[i] + 1;
            if (strcmp(s, "9") == 0 || strcasecmp(s, "SIGKILL") == 0) sig = SIGKILL;
            else if (strcmp(s, "15") == 0 || strcasecmp(s, "SIGTERM") == 0) sig = SIGTERM;
            else if (strcmp(s, "2") == 0 || strcasecmp(s, "SIGINT") == 0) sig = SIGINT;
            else {
                char *end;
                long v = strtol(s, &end, 10);
                if (*end == '\0') sig = (int)v;
                else {
                    fprintf(stderr, "kill: unknown signal: %s\n", s);
                    return 1;
                }
            }
        } else break;
    }
    if (i >= argc) {
        fprintf(stderr, "Usage: kill [-signal] pid...\n");
        return 1;
    }
    for (; i < argc; i++) {
        char *end;
        long v = strtol(argv[i], &end, 10);
        if (*end != '\0') {
            fprintf(stderr, "kill: invalid pid: %s\n", argv[i]);
            return 1;
        }
        pid_t pid = (pid_t)v;
        if (kill(pid, sig) < 0) {
            perror(argv[i]);
            return 1;
        }
    }
    return 0;
}
