#include "page_manager.h"
#include <iostream>

void PageManager::set_transaction_manager(TransactionManager* txn_manager) {
    this->txn_mgr = txn_manager;
}

Page& PageManager::get_page(int page_id) {
    if (pages.find(page_id) == pages.end()) {
        pages[page_id] = Page{page_id, -1, {}};
    }
    return pages[page_id];
}

void PageManager::apply_update(int page_id, const std::string& key, const std::string& value, int lsn, int tid) {
    if (txn_mgr) {
        txn_mgr->lock_manager.acquire(tid, key, LockType::EXCLUSIVE);  // lock before write
    }
    Page& page = get_page(page_id);
    page.page_lsn = lsn;
    page.data[key].emplace_back(value, lsn, tid);
}

void PageManager::undo_update(int page_id, const std::string& key, const std::string& old_value, int lsn, int tid) {
    if (txn_mgr) {
        txn_mgr->lock_manager.acquire(tid, key, LockType::EXCLUSIVE);  // lock before undoing
    }
    Page& page = get_page(page_id);
    page.page_lsn = lsn;
    page.data[key].emplace_back(old_value, lsn, tid);
}

std::string PageManager::read_latest_visible(int page_id, const std::string& key, int tid) {
    if (txn_mgr) {
        txn_mgr->lock_manager.acquire(tid, key, LockType::SHARED);  // lock before read
    }
    Page& page = get_page(page_id);
    if (page.data.find(key) == page.data.end()) return "";
    const auto& versions = page.data[key];
    int snapshot = mvcc->get_snapshot_lsn(tid);
    for (auto it = versions.rbegin(); it != versions.rend(); ++it) {
        const auto& [val, lsn, owner_tid] = *it;
        if (lsn <= snapshot) return val;
    }
    return "";
}

void PageManager::print_page(int page_id) {
    Page& page = get_page(page_id);
    std::cout << "Page ID: " << page_id << ", LSN: " << page.page_lsn << std::endl;
    for (const auto& [key, versions] : page.data) {
        std::cout << "  Key: " << key << " → ";
        for (const auto& [val, lsn, tid] : versions) {
            std::cout << "(" << val << ", LSN: " << lsn << ", TID: " << tid << ") ";
        }
        std::cout << std::endl;
    }
}

void PageManager::set_mvcc_manager(MVCCManager* mvcc_mgr) {
    mvcc = mvcc_mgr;
}