#!/bin/bash

DB=./build/toydb
PASSED=0
FAILED=0

test_case() {
  DESC="$1"
  INPUT="$2"
  EXPECT="$3"

  OUTPUT=$(echo -e "$INPUT" | $DB | grep -v '\[DEBUG\]' | grep -v '\[OK\]')

  if echo "$OUTPUT" | grep -q "$EXPECT"; then
    echo "[PASS] $DESC"
    PASSED=$((PASSED+1))
  else
    echo "FAIL: $DESC"
    echo "  Expected: $EXPECT"
    echo "  Got:      $OUTPUT"
    FAILED=$((FAILED+1))
  fi
}

# Reset DB state
rm -f logfile.jsonl

# --- Correctness Tests ---

test_case "Basic transaction commit" "begin\nput a 1\ncommit\nrecover\nget a\nexit" "a = 1"

test_case "Abort discards update" "begin\nput x 42\nabort\nrecover\nget x\nexit" "x = NULL"

test_case "Multiple transactions with commit" "begin\nput foo 111\ncommit\nbegin\nput bar 222\ncommit\nrecover\nget foo\nget bar\nexit" "bar = 222"

test_case "Overwritten value visible post-commit" "begin\nput z 5\ncommit\nbegin\nput z 6\ncommit\nrecover\nget z\nexit" "z = 6"

# --- Locking Tests ---

test_case "Exclusive lock blocks second transaction" $'
begin\nput a 10\n'\
'before second begin\n'\
'begin\nput a 11\n'\
'commit\nrecover\nget a\nexit' "a = 11"

test_case "Shared read locks coexist" $'
begin\nget y\n'\
'before second begin\n'\
'begin\nget y\n'\
'commit\nexit' "y = NULL"

test_case "Exclusive lock blocks shared read" $'
begin\nput locky 9\n'\
'before second begin\n'\
'begin\nget locky\n'\
'commit\nrecover\nget locky\nexit' "locky = 9"

# --- Recovery Tests ---

test_case "Recovery restores visible state" $'
begin\nput foo recovery\ncommit\nrecover\nget foo\nexit' "foo = recovery"

echo ""
echo "Tests passed: $PASSED"
echo "Tests failed: $FAILED"
exit $FAILED
