#include "lock_manager.h"

void LockManager::acquire(int txn_id, const std::string& key, LockType type) {
    std::unique_lock<std::mutex> lock(mtx);
    LockRequest req{txn_id, type};

    while (!is_compatible(key, type, txn_id)) {
        cv.wait(lock);
    }

    lock_table[key].push_back(req);
    txn_locks[txn_id].insert(key);
}

void LockManager::release_all(int txn_id) {
    std::lock_guard<std::mutex> lock(mtx);
    if (txn_locks.count(txn_id)) {
        for (const std::string& key : txn_locks[txn_id]) {
            auto& requests = lock_table[key];
            requests.erase(std::remove_if(requests.begin(), requests.end(), 
                [&](const LockRequest& r) { return r.txn_id == txn_id; }), requests.end());
        }
        txn_locks.erase(txn_id);
        cv.notify_all();
    }
}

bool LockManager::is_compatible(const std::string& key, LockType type, int txn_id) {
    const auto& requests = lock_table[key];
    for (const auto& req : requests) {
        if (req.txn_id != txn_id) {
            if (req.type == LockType::EXCLUSIVE || type == LockType::EXCLUSIVE) {
                return false;
            }
        }
    }
    return true;
}