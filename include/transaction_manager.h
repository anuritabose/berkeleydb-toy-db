// TransactionManager.h
#pragma once
#include <unordered_map>
#include <set>

enum TxnState {
    ACTIVE,
    COMMITTED,
    ABORTED
};

class TransactionManager {
private:
    int next_tid = 1;
    std::unordered_map<int, TxnState> txn_table;
    std::unordered_map<int, int> last_lsn_table;

public:
    int begin_transaction();
    void commit_transaction(int tid);
    void abort_transaction(int tid);

    void set_last_lsn(int tid, int lsn);
    int get_last_lsn(int tid);

    TxnState get_state(int tid);
    std::set<int> get_active_transactions();
};
