#pragma once

#include "vll/analysis/AnalysisInterfaces.h"

namespace vll::analysis {

class VoiceLeadingEvaluator final : public IVoiceLeadingEvaluator {
public:
    [[nodiscard]] TransitionMetrics metrics(const VoiceAssignment& assignment) const override;
    [[nodiscard]] AttemptResult evaluate(
        const VoiceAssignment& assignment,
        const ExerciseConstraint& constraints) const override;
};

} // namespace vll::analysis
