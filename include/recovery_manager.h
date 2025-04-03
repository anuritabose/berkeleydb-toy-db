#pragma once
#include "logger.h"
#include "page_manager.h"
#include "transaction_manager.h"

class RecoveryManager {
private:
    Logger& logger;
    PageManager& page_manager;
    TransactionManager& txn_manager;

public:
    RecoveryManager(Logger& log, PageManager& pm, TransactionManager& tm);

    void recover();

private:
    void analysis(const std::vector<LogRecord>& logs, std::set<int>& dirty_pages, std::set<int>& uncommitted_txns);
    void redo(const std::vector<LogRecord>& logs, const std::set<int>& dirty_pages);
    void undo(const std::vector<LogRecord>& logs, const std::set<int>& uncommitted_txns);
};
