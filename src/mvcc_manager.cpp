#include "mvcc_manager.h"

void MVCCManager::register_txn(int tid, int start_lsn) {
    txn_start_time[tid] = start_lsn;
}

void MVCCManager::commit_txn(int tid) {
    committed_txns.insert(tid);
}

int MVCCManager::get_snapshot_lsn(int tid) const {
    auto it = txn_start_time.find(tid);
    return (it != txn_start_time.end()) ? it->second : -1;
}

bool MVCCManager::is_committed(int tid) const {
    return committed_txns.count(tid) > 0;
}
