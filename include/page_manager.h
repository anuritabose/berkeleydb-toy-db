#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <tuple>
#include "mvcc_manager.h"

// Each entry has multiple versions: (value, version_lsn, tid)
typedef std::tuple<std::string, int, int> VersionedValue; // value, lsn, tid

struct Page {
    int page_id;
    int page_lsn;
    std::unordered_map<std::string, std::vector<VersionedValue>> data;
};

class PageManager {
private:
    std::unordered_map<int, Page> pages; // page_id → Page
    MVCCManager* mvcc = nullptr;

public:
    Page& get_page(int page_id);

    void apply_update(int page_id, const std::string& key, const std::string& value, int lsn, int tid);
    void undo_update(int page_id, const std::string& key, const std::string& old_value, int lsn, int tid);

    std::string read_latest_visible(int page_id, const std::string& key, int tid);

    void print_page(int page_id);
    void set_mvcc_manager(MVCCManager* mvcc_mgr);
};
