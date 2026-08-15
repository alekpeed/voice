#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace vll {

enum class LogLevel { Debug, Info, Warning, Error };

class Logger {
public:
    explicit Logger(std::filesystem::path filePath = {});
    void write(LogLevel level, const std::string& component, const std::string& message);

private:
    std::mutex mutex_;
    std::ofstream file_;
};

} // namespace vll
