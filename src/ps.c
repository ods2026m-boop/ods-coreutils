#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: ps\n");
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "ps: unknown option: %s\n", argv[i]);
            return 1;
        }
    }
    printf("%5s %5s %-20s %s\n", "PID", "STATE", "CMD", "NAME");
    DIR *d = opendir("/proc");
    if (!d) {
        perror("/proc");
        return 1;
    }
    struct dirent *ent;
    while ((ent = readdir(d))) {
        char *end;
        long pid = strtol(ent->d_name, &end, 10);
        if (*end != '\0') continue;
        char stat_path[256], cmdline_path[256];
        snprintf(stat_path, sizeof(stat_path), "/proc/%ld/stat", pid);
        snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%ld/cmdline", pid);
        FILE *f = fopen(stat_path, "r");
        if (!f) continue;
        char buf[4096];
        if (!fgets(buf, sizeof(buf), f)) {
            fclose(f);
            continue;
        }
        fclose(f);
        char state = '?';
        char name[256] = {0};
        sscanf(buf, "%*d (%255[^)]) %c", name, &state);
        if (name[0] == ' ') memmove(name, name + 1, strlen(name));
        f = fopen(cmdline_path, "r");
        char cmdline[4096] = {0};
        if (f) {
            size_t n = fread(cmdline, 1, sizeof(cmdline) - 1, f);
            fclose(f);
            for (size_t j = 0; j < n; j++) {
                if (cmdline[j] == '\0') cmdline[j] = ' ';
            }
        }
        printf("%5ld %5c %-20s %s\n", pid, state, name, cmdline);
    }
    closedir(d);
    return 0;
}
