#include "transaction_manager.h"

int TransactionManager::begin_transaction() {
    int tid = next_tid++;
    txn_table[tid] = ACTIVE;
    last_lsn_table[tid] = -1;
    return tid;
}

void TransactionManager::commit_transaction(int tid) {
    txn_table[tid] = COMMITTED;
}

void TransactionManager::abort_transaction(int tid) {
    txn_table[tid] = ABORTED;
}

void TransactionManager::set_last_lsn(int tid, int lsn) {
    last_lsn_table[tid] = lsn;
}

int TransactionManager::get_last_lsn(int tid) {
    return last_lsn_table.count(tid) ? last_lsn_table[tid] : -1;
}

TxnState TransactionManager::get_state(int tid) {
    return txn_table.count(tid) ? txn_table[tid] : ABORTED;
}

std::set<int> TransactionManager::get_active_transactions() {
    std::set<int> active;
    for (const auto& [tid, state] : txn_table) {
        if (state == ACTIVE)
            active.insert(tid);
    }
    return active;
}
