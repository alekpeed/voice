#pragma once

#include "vll/exercise/IExerciseEngine.h"

namespace vll::exercise {

class DeterministicExerciseGenerator final : public IExerciseEngine {
public:
    [[nodiscard]] ExercisePrompt generate(const std::string& exerciseId,
                                          std::uint64_t seed) const override;
    [[nodiscard]] ExercisePrompt generate(const ExerciseGenerationRequest& request) const;
    [[nodiscard]] AttemptResult submit(const ExercisePrompt& prompt,
                                       const Sonority& response) const override;
};

} // namespace vll::exercise
