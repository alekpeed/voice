#pragma once

#include "vll/core/Types.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace vll::lab {

struct LabHarmony {
    std::string symbol;
    std::vector<int> pitchClasses;
};

struct LabRequest {
    std::string id;
    std::vector<LabHarmony> harmony;
    Sonority initialVoicing;
    int voiceCount{4};
    std::vector<std::optional<Pitch>> sopranoLine;
    std::vector<std::optional<Pitch>> bassLine;
    std::vector<std::vector<Pitch>> requiredInnerLines;
};

struct LabRealization {
    bool complete{false};
    std::vector<Sonority> voicings;
    std::vector<VoicePath> voicePaths;
    int totalDisplacement{0};
    std::string error;
};

class ReharmonizationLab final {
public:
    [[nodiscard]] LabRealization realize(const LabRequest& request) const;
    bool save(const std::string& name, LabRequest request);
    [[nodiscard]] std::optional<LabRequest> load(const std::string& name) const;
    [[nodiscard]] std::vector<std::string> savedNames() const;

private:
    std::unordered_map<std::string, LabRequest> saved_;
};

} // namespace vll::lab
