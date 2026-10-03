#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

static int max_depth = 0;

static void tree(const char *path, const char *prefix, int is_last, int depth)
{
    (void)is_last;
    if (max_depth && depth > max_depth) return;
    DIR *d = opendir(path);
    if (!d) {
        perror(path);
        return;
    }
    struct dirent **entries;
    int n = scandir(path, &entries, NULL, alphasort);
    if (n < 0) {
        closedir(d);
        return;
    }
    for (int i = 0; i < n; i++) {
        if (entries[i]->d_name[0] == '.' &&
            (entries[i]->d_name[1] == '\0' ||
             (entries[i]->d_name[1] == '.' && entries[i]->d_name[2] == '\0'))) {
            free(entries[i]);
            continue;
        }
        int last = (i == n - 1);
        printf("%s%s%s\n", prefix, last ? "`-- " : "|-- ", entries[i]->d_name);
        struct stat st;
        char full[4096];
        snprintf(full, sizeof(full), "%s/%s", path, entries[i]->d_name);
        if (stat(full, &st) == 0 && S_ISDIR(st.st_mode)) {
            char new_prefix[4096];
            snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, last ? "    " : "|   ");
            tree(full, new_prefix, last, depth + 1);
        }
        free(entries[i]);
    }
    free(entries);
    closedir(d);
}

int main(int argc, char *argv[])
{
    int i = 1;
    for (; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: tree [-L depth] [dir]\n");
            return 0;
        } else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            max_depth = atoi(argv[++i]);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "tree: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    const char *dir = argv[i];
    if (!dir) dir = ".";
    printf("%s\n", dir);
    tree(dir, "", 1, 1);
    return 0;
}
