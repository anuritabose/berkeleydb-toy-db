#include "recovery_manager.h"
#include <iostream>

RecoveryManager::RecoveryManager(Logger& log, PageManager& pm, TransactionManager& tm)
    : logger(log), page_manager(pm), txn_manager(tm) {}

void RecoveryManager::recover() {
    std::vector<LogRecord> logs = logger.read_logs();
    std::set<int> dirty_pages;
    std::set<int> uncommitted_txns;

    analysis(logs, dirty_pages, uncommitted_txns);
    redo(logs, dirty_pages);
    undo(logs, uncommitted_txns);
}

void RecoveryManager::analysis(const std::vector<LogRecord>& logs, std::set<int>& dirty_pages, std::set<int>& uncommitted_txns) {
    for (const auto& log : logs) {
        if (log.type == BEGIN) {
            txn_manager.set_last_lsn(log.tid, log.lsn);
            uncommitted_txns.insert(log.tid);
        } else if (log.type == COMMIT || log.type == ABORT) {
            uncommitted_txns.erase(log.tid);
        } else if (log.type == UPDATE || log.type == CLR) {
            dirty_pages.insert(log.page_id);
            txn_manager.set_last_lsn(log.tid, log.lsn);
        }
    }
}

void RecoveryManager::redo(const std::vector<LogRecord>& logs, const std::set<int>& dirty_pages) {
    for (const auto& log : logs) {
        if ((log.type == UPDATE || log.type == CLR) && dirty_pages.count(log.page_id)) {
            Page& page = page_manager.get_page(log.page_id);
            if (page.page_lsn < log.lsn) {
                page_manager.apply_update(log.page_id, log.key, log.after_val, log.lsn, log.tid);
                std::cout << "Redo applied: LSN " << log.lsn << std::endl;
            }
        }
    }
}

void RecoveryManager::undo(const std::vector<LogRecord>& logs, const std::set<int>& uncommitted_txns) {
    for (auto it = logs.rbegin(); it != logs.rend(); ++it) {
        const LogRecord& log = *it;
        if (uncommitted_txns.count(log.tid) && log.type == UPDATE) {
            page_manager.undo_update(log.page_id, log.key, log.before_val, log.lsn + 100000, log.tid);
            std::cout << "Undo applied: LSN " << log.lsn << " (undo LSN " << log.lsn + 100000 << ")" << std::endl;
        }
    }
}
