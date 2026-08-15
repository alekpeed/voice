#pragma once

#include "vll/core/Types.h"
#include <cstdint>
#include <string>

namespace vll::exercise {

struct ExercisePrompt {
    std::string id;
    std::string instruction;
    ExerciseConstraint constraints;
    std::uint64_t seed{0};
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
