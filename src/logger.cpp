#include "logger.h"
#include <fstream>
#include <iostream>

Logger::Logger(const std::string& log_filename) : filename(log_filename) {
    log_file.open(filename, std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
    }
}

Logger::~Logger() {
    if (log_file.is_open()) log_file.close();
}

int Logger::write_log(const LogRecord& record) {
    LogRecord temp = record;
    temp.lsn = next_lsn++;

    nlohmann::json j = to_json(temp);
    log_file << j.dump() << std::endl;
    log_file.flush();
    return temp.lsn;

}

std::vector<LogRecord> Logger::read_logs() {
    std::vector<LogRecord> logs;
    std::ifstream infile(filename);
    std::string line;
    while (std::getline(infile, line)) {
        nlohmann::json j = nlohmann::json::parse(line);
        logs.push_back(from_json(j));
    }
    return logs;
}

nlohmann::json Logger::to_json(const LogRecord& record) {
    return {
        {"lsn", record.lsn},
        {"tid", record.tid},
        {"type", record.type},
        {"page_id", record.page_id},
        {"key", record.key},
        {"before_val", record.before_val},
        {"after_val", record.after_val}
    };
}

LogRecord Logger::from_json(const nlohmann::json& j) {
    return {
        j.at("lsn"),
        j.at("tid"),
        static_cast<LogType>(j.at("type")),
        j.at("page_id"),
        j.at("key"),
        j.at("before_val"),
        j.at("after_val")
    };
}
