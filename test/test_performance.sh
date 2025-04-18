#!/bin/bash

DB=./build/toydb
mkdir -p perf_logs

LOCK_CSV="perf_logs/lock_contention.csv"
MVCC_CSV="perf_logs/mvcc_scaling.csv"
THROUGHPUT_CSV="perf_logs/throughput.csv"
WAL_CSV="perf_logs/wal_size.csv"
RECOVERY_CSV="perf_logs/recovery_latency.csv"
UNDO_CSV="perf_logs/undo_cost.csv"

echo "num_conflicts,time_ms" > "$LOCK_CSV"
echo "num_versions,txn_time_ms" > "$MVCC_CSV"
echo "load_size,workload_type,ops_per_sec" > "$THROUGHPUT_CSV"
echo "load_size,wal_size_bytes" > "$WAL_CSV"
echo "load_size,recovery_time_ms" > "$RECOVERY_CSV"
echo "num_uncommitted,recovery_time_ms" > "$UNDO_CSV"

now_ms() {
  python3 -c 'import time; print(int(time.time() * 1000))'
}

# Load sizes for tests
LOAD_SIZES=(10000 14444 18888 23333 27777 32222 36666 41111 45555 50000)

# --- WAL + Recovery ---
for N in "${LOAD_SIZES[@]}"; do
  echo "begin" > temp_input.txt
  for ((i = 0; i < N; i++)); do
    echo "put k$i v$i" >> temp_input.txt
  done
  echo "commit" >> temp_input.txt
  echo "exit" >> temp_input.txt

  $DB < temp_input.txt > /dev/null

  WAL_SIZE=$(stat -f%z logfile.jsonl 2>/dev/null || echo 0)
  echo "$N,$WAL_SIZE" >> "$WAL_CSV"

  START=$(now_ms)
  echo -e "recover\nexit" | $DB > /dev/null
  END=$(now_ms)
  echo "$N,$((END - START))" >> "$RECOVERY_CSV"
done


# --- Locking overhead ---
for C in 1 5 10 20; do
  echo "begin" > temp_input.txt
  for ((i = 0; i < C; i++)); do
    echo "put sharedkey v$i" >> temp_input.txt
    echo "commit" >> temp_input.txt
    echo "begin" >> temp_input.txt
  done
  echo "exit" >> temp_input.txt

  START=$(now_ms)
  $DB < temp_input.txt > /dev/null
  END=$(now_ms)
  echo "$C,$((END - START))" >> "$LOCK_CSV"
done


# --- MVCC memory usage ---
for V in 10000 20000 30000 40000 50000; do
  echo "begin" > temp_input.txt
  for ((i = 0; i < V; i++)); do
    echo "put mvcc_key v$i" >> temp_input.txt
    echo "commit" >> temp_input.txt
    echo "begin" >> temp_input.txt
  done
  echo "sleep" >> temp_input.txt
  echo "exit" >> temp_input.txt

  ./build/toydb < temp_input.txt &
  DB_PID=$!

  # Give enough time for memory to accumulate and process to settle
  sleep 2

  MEM_USAGE=$(ps -o rss= -p "$DB_PID" 2>/dev/null | tail -n1 | xargs)
  if [ -z "$MEM_USAGE" ]; then
    MEM_USAGE="0"
  fi

  echo "$V,$MEM_USAGE" >> "$MVCC_CSV"
  kill $DB_PID 2>/dev/null
done




# --- Undo cost ---
for U in 100 500 1000 2000; do
  echo "begin" > temp_input.txt
  for ((i = 0; i < U; i++)); do
    echo "put undo_key$i val" >> temp_input.txt
  done
  echo "recover" >> temp_input.txt
  echo "exit" >> temp_input.txt

  START=$(now_ms)
  $DB < temp_input.txt > /dev/null
  END=$(now_ms)
  echo "$U,$((END - START))" >> "$UNDO_CSV"
done


# --- Throughput ---
for SIZE in 10000 14444 18888 23333 27777 32222 36666 41111 45555 50000; do
  for TYPE in write-heavy read-heavy balanced; do
    echo "begin" > temp_input.txt
    for ((i = 0; i < SIZE; i++)); do
      case $TYPE in
        write-heavy)
          echo "put k$i v$i" >> temp_input.txt
          ;;
        read-heavy)
          echo "get k$((i % 100))" >> temp_input.txt
          ;;
        balanced)
          if ((i % 2 == 0)); then
            echo "put k$i v$i" >> temp_input.txt
          else
            echo "get k$((i - 1))" >> temp_input.txt
          fi
          ;;
      esac
    done
    echo "commit" >> temp_input.txt
    echo "exit" >> temp_input.txt

    START=$(now_ms)
    $DB < temp_input.txt > /dev/null
    END=$(now_ms)
    DURATION=$((END - START))

    if [ "$DURATION" -gt 0 ]; then
      OPS=$SIZE
      [ "$TYPE" = "read-heavy" ] && OPS=$((SIZE * 2))
      OPS_PER_SEC=$(echo "scale=2; 1000 * $OPS / $DURATION" | bc)
    else
      OPS_PER_SEC="N/A"
    fi

    echo "$SIZE,$TYPE,$OPS_PER_SEC" >> "$THROUGHPUT_CSV"
  done
done

rm -f temp_input.txt
echo -e "\nBerkeleyDB Toy performance test complete. CSVs saved to perf_logs/"
