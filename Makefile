CC ?= gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700
LDFLAGS :=
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
SRCS := src/echo.c src/pwd.c src/cat.c src/head.c src/tail.c \
	src/ls.c src/du.c src/df.c src/mkdir.c src/rmdir.c \
	src/touch.c src/cp.c src/mv.c src/rm.c src/chmod.c \
	src/chown.c src/grep.c src/find.c src/tree.c src/ps.c \
	src/top.c src/kill.c src/less.c src/nano.c
BINS := $(SRCS:src/%.c=bin/%)
all: $(BINS)
bin/%: src/%.c
	mkdir -p bin
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
clean:
	rm -f $(BINS)
test: $(BINS)
	@bash tests/integration/test_all.sh

install: $(BINS)
	install -d "$(DESTDIR)$(BINDIR)"
	for bin in $(BINS); do \
		install -m 755 "$$bin" "$(DESTDIR)$(BINDIR)/$$(basename "$$bin")"; \
	done
.PHONY: all clean test install
