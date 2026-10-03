#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>

static struct termios orig;
static char **lines = NULL;
static int num_lines = 0;
static int cap_lines = 0;
static int cursor_x = 0, cursor_y = 0;
static int scroll = 0;
static const char *path = NULL;

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

static void add_line(const char *text)
{
    if (num_lines >= cap_lines) {
        cap_lines = cap_lines ? cap_lines * 2 : 16;
        lines = realloc(lines, sizeof(char *) * (size_t)cap_lines);
    }
    lines[num_lines++] = strdup(text);
}

static void draw(void)
{
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    int rows = ws.ws_row;
    printf("\033[H\033[J");
    for (int i = scroll; i < num_lines && i < scroll + rows - 1; i++) {
        fputs(lines[i], stdout);
        putchar('\n');
    }
    printf("\033[%d;%dH", cursor_y - scroll + 1, cursor_x + 1);
    fflush(stdout);
}

static void save_file(const char *p)
{
    FILE *f = fopen(p, "w");
    if (!f) {
        perror(p);
        return;
    }
    for (int i = 0; i < num_lines; i++) {
        fputs(lines[i], f);
        fputc('\n', f);
    }
    fclose(f);
}

int main(int argc, char *argv[])
{
    init_term();
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: nano [file]\n");
        return 0;
    }
    path = argc > 1 ? argv[1] : NULL;
    if (path) {
        FILE *f = fopen(path, "r");
        if (f) {
            char buf[4096];
            while (fgets(buf, sizeof(buf), f)) {
                buf[strcspn(buf, "\n")] = '\0';
                add_line(buf);
            }
            fclose(f);
        }
    }
    if (num_lines == 0) add_line("");

    draw();

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;
        if (c == 1) {
            cursor_x = 0;
        } else if (c == 5) {
            cursor_x = (int)strlen(lines[cursor_y]);
        } else if (c == 4) {
            if (cursor_x < (int)strlen(lines[cursor_y])) {
                memmove(lines[cursor_y] + cursor_x, lines[cursor_y] + cursor_x + 1,
                        strlen(lines[cursor_y]) - (size_t)cursor_x);
            }
        } else if (c == 21) {
            free(lines[cursor_y]);
            for (int i = cursor_y; i < num_lines - 1; i++) lines[i] = lines[i + 1];
            num_lines--;
            if (cursor_y >= num_lines) cursor_y = num_lines - 1;
            cursor_x = 0;
        } else if (c == 11) {
            free(lines[cursor_y]);
            for (int i = cursor_y; i < num_lines - 1; i++) lines[i] = lines[i + 1];
            num_lines--;
            if (cursor_y >= num_lines) cursor_y = num_lines - 1;
            cursor_x = 0;
        } else if (c == 12) {
        } else if (c == 19) {
            if (path) save_file(path);
        } else if (c == 17) {
            break;
        } else if (c == 127 || c == 8) {
            if (cursor_x > 0) {
                memmove(lines[cursor_y] + cursor_x - 1, lines[cursor_y] + cursor_x,
                        strlen(lines[cursor_y]) - (size_t)cursor_x + 1);
                cursor_x--;
            }
        } else if (c == '\n') {
            char *new_line = strdup(lines[cursor_y] + (size_t)cursor_x);
            lines[cursor_y][cursor_x] = '\0';
            for (int i = num_lines; i > cursor_y + 1; i--) lines[i] = lines[i - 1];
            lines[cursor_y + 1] = new_line;
            num_lines++;
            cursor_y++;
            cursor_x = 0;
        } else if (c >= 32 && c <= 126) {
            size_t len = strlen(lines[cursor_y]);
            lines[cursor_y] = realloc(lines[cursor_y], len + 2);
            memmove(lines[cursor_y] + cursor_x + 1, lines[cursor_y] + cursor_x, len - (size_t)cursor_x + 1);
            lines[cursor_y][cursor_x] = c;
            cursor_x++;
        } else if (c == 27) {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) break;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) break;
            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    if (cursor_y > 0) {
                        cursor_y--;
                        if (cursor_x > (int)strlen(lines[cursor_y])) cursor_x = (int)strlen(lines[cursor_y]);
                    }
                } else if (seq[1] == 'B') {
                    if (cursor_y < num_lines - 1) {
                        cursor_y++;
                        if (cursor_x > (int)strlen(lines[cursor_y])) cursor_x = (int)strlen(lines[cursor_y]);
                    }
                } else if (seq[1] == 'C') {
                    if (cursor_x < (int)strlen(lines[cursor_y])) cursor_x++;
                } else if (seq[1] == 'D') {
                    if (cursor_x > 0) cursor_x--;
                }
            }
        }
        draw();
    }
    for (int i = 0; i < num_lines; i++) free(lines[i]);
    free(lines);
    return 0;
}
