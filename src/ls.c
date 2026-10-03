#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

static int opt_l = 0;
static int opt_a = 0;

static void print_long(const char *path, const char *name)
{
    struct stat st;
    char full[4096];
    snprintf(full, sizeof(full), "%s/%s", path, name);
    if (stat(full, &st) < 0) {
        perror(full);
        return;
    }
    char perms[11];
    perms[0] = S_ISDIR(st.st_mode) ? 'd' : (S_ISLNK(st.st_mode) ? 'l' : '-');
    perms[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perms[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
    printf("%s %3lu %s %s %8lld %s %s\n",
           perms,
           (unsigned long)st.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?",
           (unsigned long long)st.st_size,
           timebuf,
           name);
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: ls [-l] [-a] [dir]\n");
            return 0;
        } else if (strcmp(argv[i], "-l") == 0) opt_l = 1;
        else if (strcmp(argv[i], "-a") == 0) opt_a = 1;
        else if (argv[i][0] == '-') {
            fprintf(stderr, "ls: unknown option: %s\n", argv[i]);
            return 1;
        } else break;
    }
    const char *dir = argv[i];
    if (!dir) dir = ".";
    DIR *d = opendir(dir);
    if (!d) {
        perror(dir);
        return 1;
    }
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!opt_a && ent->d_name[0] == '.') continue;
        if (opt_l) print_long(dir, ent->d_name);
        else puts(ent->d_name);
    }
    closedir(d);
    return 0;
}
