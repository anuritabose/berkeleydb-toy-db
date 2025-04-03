#pragma once
#include <unordered_map>
#include <set>

class MVCCManager {
private:
    std::unordered_map<int, int> txn_start_time; // tid → start_lsn (snapshot)
    std::set<int> committed_txns;

public:
    void register_txn(int tid, int start_lsn);
    void commit_txn(int tid);

    int get_snapshot_lsn(int tid) const;
    bool is_committed(int tid) const;
};
