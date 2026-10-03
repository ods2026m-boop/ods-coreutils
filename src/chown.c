#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

static uid_t parse_user(const char *s)
{
    char *end;
    long v = strtol(s, &end, 10);
    if (*end == '\0') return (uid_t)v;
    struct passwd *pw = getpwnam(s);
    return pw ? pw->pw_uid : (uid_t)-1;
}

static gid_t parse_group(const char *s)
{
    char *end;
    long v = strtol(s, &end, 10);
    if (*end == '\0') return (gid_t)v;
    struct group *gr = getgrnam(s);
    return gr ? gr->gr_gid : (gid_t)-1;
}

int main(int argc, char *argv[])
{
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: chown [user][:group] file\n");
        return 0;
    }
    if (argc != 3) {
        fprintf(stderr, "Usage: chown [user][:group] file\n");
        return 1;
    }
    uid_t uid = (uid_t)-1;
    gid_t gid = (gid_t)-1;
    const char *u = argv[1];
    const char *colon = strchr(u, ':');
    char ubuf[256] = {0};
    char gbuf[256] = {0};
    if (colon) {
        if (u == colon) {
            snprintf(gbuf, sizeof(gbuf), "%s", colon + 1);
            gid = parse_group(gbuf);
        } else if (colon[1] == '\0') {
            snprintf(ubuf, sizeof(ubuf), "%.*s", (int)(colon - u), u);
            uid = parse_user(ubuf);
        } else {
            snprintf(ubuf, sizeof(ubuf), "%.*s", (int)(colon - u), u);
            snprintf(gbuf, sizeof(gbuf), "%s", colon + 1);
            uid = parse_user(ubuf);
            gid = parse_group(gbuf);
        }
    } else {
        snprintf(ubuf, sizeof(ubuf), "%s", u);
        uid = parse_user(ubuf);
    }
    if (chown(argv[2], uid, gid) < 0) {
        perror(argv[2]);
        return 1;
    }
    return 0;
}
