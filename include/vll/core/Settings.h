#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace vll {

class Settings {
public:
    [[nodiscard]] static Settings defaults();
    [[nodiscard]] static std::optional<Settings> load(const std::filesystem::path& path);
    [[nodiscard]] bool save(const std::filesystem::path& path) const;

    [[nodiscard]] std::string value(const std::string& key) const;
    [[nodiscard]] bool set(const std::string& key, const std::string& value);

private:
    std::unordered_map<std::string, std::string> values_;
};

} // namespace vll
