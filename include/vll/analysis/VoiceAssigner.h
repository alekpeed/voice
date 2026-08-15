#pragma once

#include "vll/analysis/AnalysisInterfaces.h"

namespace vll::analysis {

struct VoiceAssignmentWeights {
    double pitchDistance{1.0};
    double commonToneBonus{4.0};
    double crossingPenalty{12.0};
    double leapPenalty{2.0};
    double ambiguityTolerance{1.0};
};

class VoiceAssigner final : public IVoiceAssigner {
public:
    explicit VoiceAssigner(VoiceAssignmentWeights weights = {});

    [[nodiscard]] VoiceAssignment assign(
        const Sonority& source,
        const Sonority& destination,
        const ExerciseConstraint& constraints) const override;

    [[nodiscard]] VoiceAssignmentWeights weights() const noexcept;

private:
    VoiceAssignmentWeights weights_;
};

} // namespace vll::analysis
