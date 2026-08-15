#include "vll/core/Settings.h"

#include <fstream>
#include <sstream>

namespace vll {
namespace {

bool validSetting(const std::string& key, const std::string& value) {
    if (key == "audio.a4_hz") {
        try {
            const double frequency = std::stod(value);
            return frequency >= 400.0 && frequency <= 480.0;
        } catch (...) { return false; }
    }
    if (key == "midi.chord_window_ms") {
        try {
            const int milliseconds = std::stoi(value);
            return milliseconds >= 0 && milliseconds <= 500;
        } catch (...) { return false; }
    }
    if (key == "ui.notation_scale") {
        try {
            const double scale = std::stod(value);
            return scale >= 0.5 && scale <= 2.0;
        } catch (...) { return false; }
    }
    return !key.empty() && key.find('=') == std::string::npos &&
           value.find('\n') == std::string::npos;
}

} // namespace

Settings Settings::defaults() {
    Settings settings;
    settings.values_ = {
        {"audio.a4_hz", "440.0"},
        {"audio.preset", "concert-grand-natural"},
        {"midi.chord_window_ms", "80"},
        {"ui.notation_scale", "1.0"},
        {"ui.active_route", "Home"}
    };
    return settings;
}

std::optional<Settings> Settings::load(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) return std::nullopt;

    Settings settings = defaults();
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line.front() == '#') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos) return std::nullopt;
        if (!settings.set(line.substr(0, separator), line.substr(separator + 1))) {
            return std::nullopt;
        }
    }
    return settings;
}

bool Settings::save(const std::filesystem::path& path) const {
    std::ofstream output(path, std::ios::trunc);
    if (!output) return false;
    for (const auto& [key, value] : values_) output << key << '=' << value << '\n';
    return static_cast<bool>(output);
}

std::string Settings::value(const std::string& key) const {
    const auto found = values_.find(key);
    return found == values_.end() ? std::string{} : found->second;
}

bool Settings::set(const std::string& key, const std::string& value) {
    if (!validSetting(key, value)) return false;
    values_[key] = value;
    return true;
}

} // namespace vll
