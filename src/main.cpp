#include "logger.h"
#include "page_manager.h"
#include "transaction_manager.h"
#include "recovery_manager.h"
#include "mvcc_manager.h"
#include <iostream>
#include <sstream>

int main() {
    Logger logger("logfile.jsonl");
    PageManager page_mgr;
    TransactionManager txn_mgr;
    MVCCManager mvcc_mgr;

    page_mgr.set_mvcc_manager(&mvcc_mgr);
    page_mgr.set_transaction_manager(&txn_mgr);

    std::string cmd;
    int current_tid = -1;

    std::cout << "Toy BerkeleyDB > ";
    while (std::getline(std::cin, cmd)) {
        if (cmd.empty()) continue;

        std::istringstream iss(cmd);
        std::string op;
        iss >> op;

        if (op == "begin") {
            current_tid = txn_mgr.begin_transaction();
            int lsn = logger.write_log({0, current_tid, BEGIN, -1, "", "", ""});
            mvcc_mgr.register_txn(current_tid, lsn);
            std::cout << "[OK] Transaction " << current_tid << " started\n";
        }

        else if (op == "put") {
            std::string key, value;
            iss >> key >> value;
            if (current_tid == -1) {
                std::cerr << "[ERROR] No active transaction\n";
                continue;
            }
            LogRecord update{0, current_tid, UPDATE, 1, key, "", value};
            update.lsn = logger.write_log(update);
            page_mgr.apply_update(1, key, value, update.lsn, current_tid);
            std::cout << "[OK] PUT " << key << "=" << value << "\n";
        }

        else if (op == "get") {
            std::string key;
            iss >> key;
            if (current_tid == -1) {
                std::cerr << "[ERROR] No active transaction\n";
                continue;
            }
            std::string val = page_mgr.read_latest_visible(1, key, current_tid);
            std::cout << "[OK] GET " << key << " = " << (val.empty() ? "NULL" : val) << "\n";
        }

        else if (op == "commit") {
            if (current_tid == -1) {
                std::cerr << "[ERROR] No active transaction\n";
                continue;
            }
            LogRecord commit{0, current_tid, COMMIT, -1, "", "", ""};
            logger.write_log(commit);
            txn_mgr.commit_transaction(current_tid);
            std::cout << "[OK] Transaction " << current_tid << " committed\n";
            current_tid = -1;
        }

        else if (op == "abort") {
            if (current_tid == -1) {
                std::cerr << "[ERROR] No active transaction\n";
                continue;
            }
            LogRecord abort{0, current_tid, ABORT, -1, "", "", ""};
            logger.write_log(abort);
            txn_mgr.abort_transaction(current_tid);
            std::cout << "[OK] Transaction " << current_tid << " aborted\n";
            current_tid = -1;
        }

        else if (op == "recover") {
            std::cout << "[DEBUG] Running recovery...\n";
            RecoveryManager recovery(logger, page_mgr, txn_mgr);
            recovery.recover();
            std::cout << "[OK] Recovery completed\n";
        }

        else if (op == "print") {
            page_mgr.print_page(1);
        }

        else if (op == "exit") {
            std::cout << "[EXIT] Bye!\n";
            break;
        }

        else {
            std::cerr << "[ERROR] Unknown command: " << op << "\n";
        }

        std::cout << "Toy BerkeleyDB > ";
    }

    return 0;
}
