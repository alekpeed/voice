#pragma once

#include "vll/core/Types.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace vll::exercise {

enum class ExerciseType { NearestVoicing, FixedSoprano, FixedBass, GuideTonesOnly };

struct ExerciseGenerationRequest {
    std::string exerciseId;
    std::uint64_t seed{0};
    std::optional<int> keyPitchClass;
    std::optional<int> voiceCount;
    std::optional<ExerciseType> type;
};

struct ExercisePrompt {
    std::string id;
    std::string instruction;
    ExerciseConstraint constraints;
    std::uint64_t seed{0};
    ExerciseType type{ExerciseType::NearestVoicing};
    int keyPitchClass{0};
    std::string sourceChord;
    std::string destinationChord;
    Sonority source;
    Sonority target;
    std::vector<int> allowedDestinationPitchClasses;
    std::vector<int> requiredDestinationPitchClasses;
    int minimumMidi{36};
    int maximumMidi{84};
    int maximumAdjacentSpacing{19};
    int optimalTotalDisplacement{0};
    bool valid{false};
    std::string error;
};

class IExerciseEngine {
public:
    virtual ~IExerciseEngine() = default;
    [[nodiscard]] virtual ExercisePrompt generate(const std::string& exerciseId,
                                                   std::uint64_t seed) const = 0;
    [[nodiscard]] virtual AttemptResult submit(const ExercisePrompt& prompt,
                                                const Sonority& response) const = 0;
};

} // namespace vll::exercise
