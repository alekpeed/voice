#include "vll/core/Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace vll {
namespace {

const char* nameOf(const LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace

Logger::Logger(std::filesystem::path filePath) {
    if (!filePath.empty()) file_.open(filePath, std::ios::app);
}

void Logger::write(const LogLevel level, const std::string& component,
                   const std::string& message) {
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    std::ostringstream line;
    line << std::format("{:%FT%TZ}", seconds) << ' ' << nameOf(level)
         << " [" << component << "] " << message;

    std::lock_guard lock(mutex_);
    std::clog << line.str() << '\n';
    if (file_) {
        file_ << line.str() << '\n';
        file_.flush();
    }
}

} // namespace vll
