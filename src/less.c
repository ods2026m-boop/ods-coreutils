#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>

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

static void display_lines(char *lines[], int count, int start, int page)
{
    int end = start + page;
    if (end > count) end = count;
    printf("\033[H\033[J");
    for (int i = start; i < end; i++) {
        fputs(lines[i], stdout);
    }
    printf("\033[7m--More--\033[0m");
    fflush(stdout);
}

static void search(const char *pattern, char *lines[], int count, int *pos)
{
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED) != 0) return;
    for (int i = *pos + 1; i < count; i++) {
        if (regexec(&re, lines[i], 0, NULL, 0) == 0) {
            *pos = i;
            break;
        }
    }
    regfree(&re);
}

int main(int argc, char *argv[])
{
    int argi = 1;
    for (; argi < argc; argi++) {
        if (strcmp(argv[argi], "-h") == 0 || strcmp(argv[argi], "--help") == 0) {
            printf("Usage: less [file]\n");
            return 0;
        } else if (argv[argi][0] == '-') {
            fprintf(stderr, "less: unknown option: %s\n", argv[argi]);
            return 1;
        } else break;
    }
    if (isatty(STDIN_FILENO))
        init_term();
    FILE *f = stdin;
    if (argi < argc) {
        f = fopen(argv[argi], "r");
        if (!f) {
            perror(argv[argi]);
            return 1;
        }
    }
    char buf[4096];
    char *lines[65536];
    int count = 0;
    while (fgets(buf, sizeof(buf), f) && count < 65536) {
        lines[count++] = strdup(buf);
    }
    if (f != stdin) fclose(f);

    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    int page = ws.ws_row > 1 ? ws.ws_row - 1 : 20;

    int pos = 0;
    while (pos < count) {
        display_lines(lines, count, pos, page);
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;
        if (c == 'q') break;
        else if (c == ' ' || c == '\n' || c == 'j') pos += page;
        else if (c == 'b') pos -= page;
        else if (c == 'k') pos--;
        else if (c == '/') {
            char pat[256];
            int j = 0;
            while (j < 255) {
                if (read(STDIN_FILENO, &c, 1) != 1) break;
                if (c == '\n' || c == '\r') break;
                pat[j++] = c;
            }
            pat[j] = '\0';
            search(pat, lines, count, &pos);
        }
        if (pos < 0) pos = 0;
    }
    printf("\033[H\033[J");
    for (int i = 0; i < count; i++) free(lines[i]);
    return 0;
}
