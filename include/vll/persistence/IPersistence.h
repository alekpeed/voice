#pragma once

#include "vll/core/Types.h"
#include <optional>
#include <string>
#include <vector>

namespace vll::persistence {

struct AttemptRecord {
    std::string exerciseId;
    std::string conceptId;
    TimestampMicros occurredAt{0};
    AttemptResult result;
};

class IPersistence {
public:
    virtual ~IPersistence() = default;
    virtual bool initialize() = 0;
    virtual bool saveAttempt(const AttemptRecord& attempt) = 0;
    [[nodiscard]] virtual std::vector<AttemptRecord> attemptsFor(const std::string& conceptId) const = 0;
    virtual bool saveSetting(const std::string& key, const std::string& value) = 0;
    [[nodiscard]] virtual std::optional<std::string> setting(const std::string& key) const = 0;
};

} // namespace vll::persistence
