#pragma once

#include "vll/analysis/AnalysisInterfaces.h"

#include <optional>
#include <vector>

namespace vll::exercise {

struct VoicingSearchRequest {
    Sonority source;
    std::vector<int> allowedPitchClasses;
    std::vector<int> requiredPitchClasses;
    int voiceCount{4};
    int minimumMidi{36};
    int maximumMidi{84};
    int maximumLeap{12};
    int maximumAdjacentSpacing{19};
    std::optional<Pitch> lockedBass;
    std::optional<Pitch> lockedSoprano;
};

struct VoicingSolution {
    bool found{false};
    Sonority destination;
    analysis::VoiceAssignment assignment;
    analysis::TransitionMetrics metrics;
};

class NearestVoicingSolver final {
public:
    [[nodiscard]] VoicingSolution solve(const VoicingSearchRequest& request) const;
};

} // namespace vll::exercise
