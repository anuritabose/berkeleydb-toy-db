#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"

enum LogType {
    BEGIN,
    UPDATE,
    COMMIT,
    ABORT,
    CLR
};

struct LogRecord {
    int lsn;
    int tid;
    LogType type;
    int page_id;
    std::string key;
    std::string before_val;
    std::string after_val;
};

class Logger {
private:
    int next_lsn = 1;
    std::ofstream log_file;
    std::string filename;

public:
    Logger(const std::string& log_filename);
    ~Logger();

    int write_log(const LogRecord& record);
    std::vector<LogRecord> read_logs();

    nlohmann::json to_json(const LogRecord& record);
    LogRecord from_json(const nlohmann::json& j);
};
