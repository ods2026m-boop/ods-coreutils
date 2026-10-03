#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <time.h>

static struct termios orig;

static void cleanup(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

static void init_term(void)
{
    tcgetattr(STDIN_FILENO, &orig);
    struct termios raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    atexit(cleanup);
}

static void print_procs(void)
{
    printf("%5s %5s %-20s %s\n", "PID", "STATE", "CMD", "NAME");
    DIR *d = opendir("/proc");
    if (!d) {
        perror("/proc");
        return;
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
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: top\n");
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "top: unknown option: %s\n", argv[i]);
            return 1;
        } else {
            fprintf(stderr, "top: unexpected argument: %s\n", argv[i]);
            return 1;
        }
    }
    init_term();
    while (1) {
        printf("\033[H\033[J");
        print_procs();
        printf("Press 'q' to quit\n");
        fflush(stdout);
        struct timespec ts = {1, 0};
        nanosleep(&ts, NULL);
        fd_set rfds;
        struct timeval tv = {0, 0};
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        if (select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) > 0) {
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1 && c == 'q') break;
        }
    }
    return 0;
}
