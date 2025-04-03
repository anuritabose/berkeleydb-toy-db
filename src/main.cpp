#include "logger.h"
#include "page_manager.h"
#include "transaction_manager.h"
#include "recovery_manager.h"
#include "mvcc_manager.h"
#include <iostream>

int main() {
    Logger logger("logfile.jsonl");
    PageManager page_mgr;
    TransactionManager txn_mgr;
    MVCCManager mvcc_mgr;

    page_mgr.set_mvcc_manager(&mvcc_mgr);

    // Simulate a new transaction
    int tid = txn_mgr.begin_transaction();

    LogRecord begin_log{0, tid, BEGIN, -1, "", "", ""};
    int begin_lsn = logger.write_log(begin_log);
    mvcc_mgr.register_txn(tid, begin_lsn);

    LogRecord update1{0, tid, UPDATE, 1, "foo", "", "bar"};
    update1.lsn = logger.write_log(update1);

    LogRecord update2{0, tid, UPDATE, 1, "baz", "", "qux"};
    update2.lsn = logger.write_log(update2);

    LogRecord commit_log{0, tid, COMMIT, -1, "", "", ""};
    commit_log.lsn = logger.write_log(commit_log);
    txn_mgr.commit_transaction(tid);


    std::cout << "\nSimulating crash and recovery...\n" << std::endl;

    RecoveryManager recovery(logger, page_mgr, txn_mgr);
    recovery.recover();

    std::cout << "\nFinal state of page 1 after recovery:\n";
    page_mgr.print_page(1);

    std::string visible_foo = page_mgr.read_latest_visible(1, "foo", tid);
    std::cout << "\nRead(foo) for tid=" << tid << " returned: " << visible_foo << std::endl;

    return 0;
}