#pragma once

#include "vll/analysis/AnalysisInterfaces.h"

#include <cstddef>
#include <vector>

namespace vll::visualization {

struct VoicePathBuildResult {
    std::vector<VoicePath> paths;
    std::vector<analysis::VoiceAssignment> assignments;
    bool complete{false};
    std::size_t failedTransition{0};
};

class VoicePathBuilder final {
public:
    explicit VoicePathBuilder(const analysis::IVoiceAssigner& assigner);

    [[nodiscard]] VoicePathBuildResult build(
        const std::vector<Sonority>& sonorities,
        const ExerciseConstraint& constraints) const;

private:
    const analysis::IVoiceAssigner& assigner_;
};

} // namespace vll::visualization
