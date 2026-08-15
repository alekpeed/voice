#pragma once

#include "vll/core/Types.h"
#include <optional>
#include <vector>

namespace vll::analysis {

struct VoiceAssignment {
    std::vector<VoiceTransition> transitions;
    double confidence{0.0};
    bool ambiguous{false};
};

class IVoiceAssigner {
public:
    virtual ~IVoiceAssigner() = default;
    [[nodiscard]] virtual VoiceAssignment assign(
        const Sonority& source,
        const Sonority& destination,
        const ExerciseConstraint& constraints) const = 0;
};

class IHarmonicAnalyzer {
public:
    virtual ~IHarmonicAnalyzer() = default;
    [[nodiscard]] virtual std::vector<ChordCandidate> analyze(
        const Sonority& sonority,
        std::optional<Pitch> tonalCenter) const = 0;
};

struct TransitionMetrics {
    int totalSemitoneDisplacement{0};
    int maximumVoiceMovement{0};
    int stationaryVoices{0};
    int semitoneMoves{0};
    int wholeStepMoves{0};
    int leaps{0};
    bool hasCrossing{false};
    bool hasOverlap{false};
    int maximumAdjacentSpacing{0};
    int bassMovement{0};
    int sopranoMovement{0};
};

class IVoiceLeadingEvaluator {
public:
    virtual ~IVoiceLeadingEvaluator() = default;
    [[nodiscard]] virtual TransitionMetrics metrics(const VoiceAssignment& assignment) const = 0;
    [[nodiscard]] virtual AttemptResult evaluate(
        const VoiceAssignment& assignment,
        const ExerciseConstraint& constraints) const = 0;
};

} // namespace vll::analysis
