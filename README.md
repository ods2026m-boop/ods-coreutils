# ods-coreutils

ODS-OS base utilities — 24 standalone C99 binaries built with GCC and
dynamically linked against the system libc, currently targeting the ODS-os
glibc-based Linux userspace.

## Build

Default compiler is GCC:

    make

You can also explicitly set the compiler with `make CC=gcc`.

All tools are built to `bin/` using normal dynamic linking against the
system libc; the current Makefile uses no explicit static-linking flag.

Build flags: `-Wall -Wextra -Werror -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700`

## Test

Run the integration test suite:

    make test

Or directly:

    bash tests/integration/test_all.sh

All non-interactive tests redirect stdin from `/dev/null` or fixture files so
the suite never hangs. `less` and `nano` are tested with `timeout 5` and
simulated key input.

## Tools and implementation scope

### Tier 1 — trivial, no filesystem writes

**echo** — prints arguments. Implements `-n` (no trailing newline).
Missing: `-e` (escape interpretation), `-E` (disable escapes, default).

**pwd** — prints current working directory via `getcwd()`. No options.

**cat** — concatenates files to stdout. Supports multiple files, stdin when no
args, and `-` for explicit stdin. No options like `-n` (line numbers) or `-b`.

### Tier 2 — basic filesystem read

**head** — prints first N lines (default 10). Supports `-n N` (both `-n1` and
`-n 1` forms). Missing: `-c` (bytes), `-q`, `-v`.

**tail** — prints last N lines (default 10). Supports `-n N` (both `-n1` and
`-n 1` forms). Missing: `-c` (bytes), `-f` (follow), `-q`, `-v`.

**ls** — lists directory contents. Supports bare (name-only), `-l` (long
format: perms, links, user, group, size, mtime, name), and `-a` (show
dotfiles). Missing: `-R` (recursive), `-t` (sort by time), `-S` (sort by
size), `-h` with `-l`, color output.

**du** — disk usage of files/directories. Supports `-h` (human-readable: B/K/M/G).
Recurses into subdirectories. Missing: `-s` (summary only), `-a` (show individual
files), `--max-depth=N`.

**df** — disk free space. Supports `-h` (human-readable). Shows one filesystem
at a time. Missing: `-T` (filesystem type), `-i` (inode info), all-filesystems
mode.

### Tier 3 — filesystem writes / structure

**mkdir** — creates directories. Supports `-p` (create parents as needed).
Missing: `-m` (set mode), `-v` (verbose).

**rmdir** — removes empty directories. No options.

**touch** — creates empty files or updates mtime. No options like `-t`
(specific time) or `-a`/`-m` (access-only / modify-only).

**cp** — copies files. Supports `-r`/`-R` (recursive directories).
Missing: `-p` (preserve mode/ownership/times), `-v` (verbose), `-f` (force),
`-i` (interactive), hard/soft link modes.

**mv** — moves/renames files. Supports same-device `rename()` and
cross-device copy+unlink for regular files. Missing: cross-device directory
moves, `-f`, `-i`, `-n`, `-v`.

**rm** — removes files. Supports `-r`/`-R` (recursive) and `-f` (force, no
error on missing). Missing: `-i` (interactive), `-v` (verbose), `-d`
(remove empty directories).

**chmod** — changes file permissions. Supports octal (e.g., `755`) and
symbolic modes (`u+x`, `go-w`, `a=r`). Missing: `-R` (recursive), `-v`
(verbose), setuid/setgid/sticky bit manipulation in symbolic mode, `X`
(conditional execute).

**chown** — changes file owner/group. Supports `user`, `user:group`, and
`:group`-only forms. Missing: `-R` (recursive), numeric-only UID/GID
assignment without name lookup.

### Tier 4 — search / text processing

**grep** — searches for patterns. Uses POSIX `<regex.h>` (`regcomp`/`regexec`).
Supports `-i` (case-insensitive), `-n` (line numbers), `-v` (invert match),
`-r`/`-R` (recursive). Regex syntax: POSIX Extended Regular Expressions (ERE).
Missing: basic regex (BRE) mode (`-E`/`-G` distinction), `-c` (count only),
`-l`/`-L` (list files with/without matches), `-o` (only matching), `-w`
(word match), `-x` (line match), `-A`/`-B`/`-C` (context lines), `--include`,
`--exclude`, null-byte output (`-Z`/`-z`).

**find** — searches for files under a directory tree. Uses `nftw()`.
Supports `-name PATTERN` (simple glob with `*` and `?`) and `-type f|d`.
Path comes before expressions (standard `find` syntax).
Missing: `-mtime`, `-size`, `-perm`, `-exec`, `-print`, `-prune`,
`-newer`, `-user`, `-group`, `-atime`, `-ctime`, `-depth`, `-maxdepth`,
`-mindepth`, `-links`, `-inum`, and many other standard predicates.

**tree** — prints directory trees. Supports ASCII tree drawing with depth
limiting via `-L N`. Missing: `-d` (directories only), `-f` (full path),
`-i` (no indentation), `-o` (output to file), color output, hidden file
filtering options, `--noreport`, `--charset`.

### Tier 5 — process monitoring / control

**ps** — lists processes by reading `/proc/<pid>/stat` and `/proc/<pid>/cmdline`.
Shows PID, state, command name, and full cmdline.
Missing: `-e`/`-A` (all processes), `-f` (full format), `-l` (long format),
`-u` (user-oriented), process selection by uid/gid, `--sort`, thread info.

**top** — live-updating process list. Simple ANSI-escape refresh (clear screen,
reprint) at 1-second intervals. Press `q` to quit.
Missing: interactive key bindings (sort columns, kill processes, renice),
cumulative mode, batch mode, I/O monitoring, memory summary, process filtering,
curses/ncurses-based TUI.

**kill** — sends signals. Supports `-9`/`-SIGKILL`, `-15`/`-SIGTERM`
(default), `-2`/`-SIGINT`, and numeric signals.
Missing: `-l` (list signals), signal names like `HUP`, `USR1`, etc. beyond
the explicitly handled ones, sending to process groups (`-SIGTERM -123`),
`pkill`-style name matching.

### Tier 6 — editor / pager

**less** — pager. Reads file into memory, displays one screen at a time.
Keys: `space`/`Enter`/`j` (forward), `b` (backward), `k` (backward one line),
`q` (quit), `/` (search forward with basic regex via POSIX `<regex.h>`).
Missing: `-N` (line numbers), `-S` (chop long lines), `-X` (no termcap init),
`-R` (raw control chars), `-i` (search ignore case), `-g` (highlight current
match), backward search (`?`), `n`/`N` (repeat search), multiple file
navigation (`:n`/`:p`), marks, editing commands, pipe support.

**nano** — minimal line editor. Opens files, allows insert/delete characters,
arrow-key navigation, enter for newline, backspace, Ctrl-Q to quit, Ctrl-S to
save, Ctrl-A (beginning of line), Ctrl-E (end of line), Ctrl-D (delete char),
Ctrl-U/Ctrl-K (delete/cut line), Ctrl-L (refresh). Uses `<termios.h>` raw mode.
Missing: syntax highlighting, search/replace (`Ctrl-\`, `Ctrl-R`), cut/paste
buffer, multiple file buffers, word wrapping, spell checking, mouse support,
status bar, help text, config file support, all standard nano keybindings
(Ctrl-X save, Ctrl-O write-out, etc.).

## Dynamic linking note

The current build uses normal dynamic system libc linking, so the binaries
depend on the libc (glibc) provided by the target Linux userspace at runtime.
There is no static-linking flag in the Makefile.

## Files

```
ods-coreutils/
  Makefile           # builds all 24 tools
  src/               # one .c file per tool
  tests/integration/ # shell-based integration tests
  POSIX              # header/syscall compliance listing
  README.md          # this file
  .gitignore
```
