#!/bin/bash

cd "$(dirname "$0")/../.."

WORK="tests/integration/work"
FAIL=0

cleanup() {
	rm -rf "$WORK"
}
trap cleanup EXIT

mkdir -p "$WORK"
cd "$WORK"

pass() {
	echo "PASS: $1"
}

fail() {
	echo "FAIL: $1"
	FAIL=$((FAIL + 1))
}

# --- echo ---
expect=$(../../../bin/echo hello)
if [ "$expect" = "hello" ]; then pass "echo hello"; else fail "echo hello"; fi

expect=$(printf 'hello' | ../../../bin/echo -n hello)
if [ "$expect" = "hello" ]; then pass "echo -n"; else fail "echo -n"; fi

# --- pwd ---
expect=$(../../../bin/pwd)
if [ "$expect" = "$PWD" ]; then pass "pwd"; else fail "pwd"; fi

# --- cat ---
echo "line1" > src.txt
echo "line2" >> src.txt
expect=$(../../../bin/cat src.txt)
if [ "$expect" = "line1
line2" ]; then pass "cat file"; else fail "cat file"; fi

expect=$(printf 'stdin\n' | ../../../bin/cat)
if [ "$expect" = "stdin" ]; then pass "cat stdin"; else fail "cat stdin"; fi

out=$(timeout 3 ../../../bin/cat --help)
if [ "$out" = "Usage: cat [file...]" ]; then pass "cat --help"; else fail "cat --help (got '$out')"; fi

rc=$(timeout 3 ../../../bin/cat -X 2>&1; echo $?)
if echo "$rc" | grep -q '1$'; then pass "cat unknown flag exit"; else fail "cat unknown flag exit (got '$rc')"; fi

# --- head ---
expect=$(../../../bin/head -n 1 src.txt)
if [ "$expect" = "line1" ]; then pass "head -n 1"; else fail "head -n 1"; fi

expect=$(../../../bin/head src.txt < /dev/null)
if [ "$expect" = "line1
line2" ]; then pass "head default"; else fail "head default"; fi

# --- tail ---
expect=$(../../../bin/tail -n 1 src.txt)
if [ "$expect" = "line2" ]; then pass "tail -n 1"; else fail "tail -n 1"; fi

# --- ls ---
mkdir -p lsdir
touch lsdir/a lsdir/b
out=$(../../../bin/ls lsdir | sort)
if [ "$out" = "a
b" ]; then pass "ls bare"; else fail "ls bare"; fi

out=$(../../../bin/ls -a lsdir | sort)
if echo "$out" | grep -q '^\.\.$'; then pass "ls -a"; else fail "ls -a"; fi

out=$(../../../bin/ls -l lsdir | grep -E '^-' | wc -l)
if [ "$out" = "2" ]; then pass "ls -l"; else fail "ls -l"; fi

# --- du ---
mkdir -p dudir/sub
echo "x" > dudir/sub/file
out=$(../../../bin/du -h dudir | awk '{print $1}')
if [ -n "$out" ]; then pass "du -h"; else fail "du -h"; fi

out=$(../../../bin/du dudir | awk '{print $1}')
if [ "$out" -gt 0 ] 2>/dev/null; then pass "du raw"; else fail "du raw"; fi

# --- df ---
out=$(../../../bin/df -h / | wc -l)
if [ "$out" -gt 1 ] 2>/dev/null; then pass "df -h"; else fail "df -h"; fi

# --- mkdir ---
../../../bin/mkdir -p mkdira/b/c
if [ -d mkdira/b/c ]; then pass "mkdir -p"; else fail "mkdir -p"; fi

# --- rmdir ---
../../../bin/rmdir mkdira/b/c
if [ ! -d mkdira/b/c ]; then pass "rmdir"; else fail "rmdir"; fi

# --- touch ---
touch tfile
../../../bin/touch tfile
if [ -f tfile ]; then pass "touch existing"; else fail "touch existing"; fi

../../../bin/touch newfile
if [ -f newfile ]; then pass "touch new"; else fail "touch new"; fi

# --- cp ---
echo "abc" > cpsrc
../../../bin/cp cpsrc cpdst
if cmp -s cpsrc cpdst; then pass "cp file"; else fail "cp file"; fi

# --- mv ---
../../../bin/mv cpdst cpdst2
if cmp -s cpsrc cpdst2; then pass "mv"; else fail "mv"; fi

# --- rm ---
../../../bin/rm cpdst2
if [ ! -f cpdst2 ]; then pass "rm"; else fail "rm"; fi

# --- chmod ---
touch chmodfile
../../../bin/chmod 644 chmodfile
mode=$(stat -c '%a' chmodfile)
if [ "$mode" = "644" ]; then pass "chmod 644"; else fail "chmod 644 (got $mode)"; fi

# --- chown ---
touch chownfile
../../../bin/chown "$(id -un):$(id -gn)" chownfile && pass "chown" || fail "chown"

# --- grep ---
echo "hello world" > grepfile
echo "foo bar" >> grepfile
out=$(../../../bin/grep hello grepfile)
if [ "$out" = "hello world" ]; then pass "grep match"; else fail "grep match"; fi

out=$(../../../bin/grep -n hello grepfile)
if echo "$out" | grep -q ':1:'; then pass "grep -n"; else fail "grep -n"; fi

out=$(../../../bin/grep -v foo grepfile)
if [ "$out" = "hello world" ]; then pass "grep -v"; else fail "grep -v"; fi

out=$(echo "line1
line2" | ../../../bin/grep line1)
if [ "$out" = "line1" ]; then pass "grep stdin"; else fail "grep stdin"; fi

../../../bin/grep zzz_not_present grepfile > /dev/null
rc=$?
if [ "$rc" -eq 1 ]; then pass "grep exit code on no match"; else fail "grep exit code on no match (got $rc)"; fi

../../../bin/grep hello grepfile > /dev/null
rc=$?
if [ "$rc" -eq 0 ]; then pass "grep exit code on match"; else fail "grep exit code on match (got $rc)"; fi

out=$(timeout 3 ../../../bin/grep --help)
if [ "$out" = "Usage: grep [-invr] pattern [file...]" ]; then pass "grep --help"; else fail "grep --help (got '$out')"; fi

rc=$(timeout 3 ../../../bin/grep -X 2>&1; echo $?)
if echo "$rc" | grep -q '1$'; then pass "grep unknown flag exit"; else fail "grep unknown flag exit (got '$rc')"; fi

# --- find ---
mkdir -p finddir/sub
touch finddir/sub/test.txt
out=$(../../../bin/find finddir -name '*.txt' | sort)
if [ "$out" = "finddir/sub/test.txt" ]; then pass "find -name"; else fail "find -name (got '$out')"; fi

out=$(../../../bin/find finddir -type d | sort)
if [ "$out" = "finddir
finddir/sub" ]; then pass "find -type d"; else fail "find -type d"; fi

# --- tree ---
out=$(../../../bin/tree finddir)
if echo "$out" | grep -q 'test.txt'; then pass "tree"; else fail "tree"; fi

# --- ps ---
out=$(../../../bin/ps | grep -E 'bin/ps|PID')
if [ -n "$out" ]; then pass "ps"; else fail "ps"; fi

# --- top ---
out=$(timeout 3 ../../../bin/top --help)
if [ "$out" = "Usage: top" ]; then pass "top --help"; else fail "top --help (got '$out')"; fi

rc=$(timeout 3 ../../../bin/top --bogus 2>&1; echo $?)
if echo "$rc" | grep -q '1$'; then pass "top unknown flag exit"; else fail "top unknown flag exit (got '$rc')"; fi

# --- kill ---
(sleep 100) > /dev/null 2>&1 &
PID=$!
../../../bin/kill -9 $PID
wait $PID 2>/dev/null || true
if ! kill -0 $PID 2>/dev/null; then pass "kill -9"; else fail "kill -9"; fi

# --- less ---
echo -e "a\nb\nc" > lessfile
echo "q" | timeout 5 ../../../bin/less lessfile > /dev/null && pass "less" || fail "less"

out=$(timeout 3 ../../../bin/less --help)
if [ "$out" = "Usage: less [file]" ]; then pass "less --help"; else fail "less --help (got '$out')"; fi

rc=$(timeout 3 ../../../bin/less -X 2>&1; echo $?)
if echo "$rc" | grep -q '1$'; then pass "less unknown flag exit"; else fail "less unknown flag exit (got '$rc')"; fi

# --- nano ---
echo "" > nanofile
echo -e "\x11" | timeout 5 ../../../bin/nano nanofile > /dev/null && pass "nano" || fail "nano"

# --- Summary ---
echo ""
echo "Tests run. Failures: $FAIL"
if [ "$FAIL" -gt 0 ]; then exit 1; fi
