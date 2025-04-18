#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>

enum class LockType {
    SHARED,
    EXCLUSIVE
};

struct LockRequest {
    int txn_id;
    LockType type;
};

class LockManager {
public:
    // Request a lock on a key. Blocks until the lock is granted.
    void acquire(int txn_id, const std::string& key, LockType type);

    // Release all locks held by a transaction
    void release_all(int txn_id);

private:
    std::mutex mtx;
    std::condition_variable cv;
    std::unordered_map<std::string, std::vector<LockRequest>> lock_table;
    std::unordered_map<int, std::unordered_set<std::string>> txn_locks;

    bool is_compatible(const std::string& key, LockType type, int txn_id);
};