#pragma once

#include "vll/analysis/AnalysisInterfaces.h"

#include <optional>
#include <string>
#include <vector>

namespace vll::analysis {

enum class PitchNamePreference { Sharps, Flats };

struct ChordFeedbackContext {
    std::string label;
    std::vector<int> guideTonePitchClasses;
};

struct FeedbackContext {
    std::optional<ChordFeedbackContext> sourceChord;
    std::optional<ChordFeedbackContext> destinationChord;
    PitchNamePreference pitchNames{PitchNamePreference::Sharps};
};

struct FeedbackReport {
    TransitionMetrics metrics;
    std::vector<Observation> observations;
};

class IDeterministicFeedbackGenerator {
public:
    virtual ~IDeterministicFeedbackGenerator() = default;
    [[nodiscard]] virtual FeedbackReport generate(
        const VoiceAssignment& assignment,
        const FeedbackContext& context = {}) const = 0;
};

class DeterministicFeedbackGenerator final : public IDeterministicFeedbackGenerator {
public:
    [[nodiscard]] FeedbackReport generate(
        const VoiceAssignment& assignment,
        const FeedbackContext& context = {}) const override;
};

} // namespace vll::analysis
