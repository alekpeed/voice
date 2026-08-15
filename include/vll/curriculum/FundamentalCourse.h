#pragma once

#include "vll/analysis/DeterministicFeedback.h"
#include "vll/core/Types.h"
#include "vll/notation/INotationRenderer.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace vll::curriculum {

struct CourseChord {
    std::string symbol;
    std::vector<int> guideTonePitchClasses;
    Sonority target;
};

struct FundamentalCourseStep {
    std::string id;
    std::string title;
    std::string instruction;
    std::vector<std::string> conceptIds;
    std::string bookExerciseId;
    std::vector<CourseChord> chords;
};

struct CourseAttemptResult {
    bool accepted{false};
    bool courseComplete{false};
    std::vector<Observation> observations;
    std::vector<analysis::FeedbackReport> transitionFeedback;
    notation::NotationDocument notation;
    std::vector<VoicePath> voicePaths;
};

class FundamentalCourseEngine final {
public:
    [[nodiscard]] static const std::vector<FundamentalCourseStep>& steps();
    [[nodiscard]] CourseAttemptResult evaluate(
        const FundamentalCourseStep& step,
        const std::vector<Sonority>& response) const;
};

class FundamentalCourseSession final {
public:
    [[nodiscard]] std::size_t currentStepIndex() const noexcept;
    [[nodiscard]] const FundamentalCourseStep* currentStep() const noexcept;
    [[nodiscard]] bool complete() const noexcept;
    [[nodiscard]] CourseAttemptResult submit(const std::vector<Sonority>& response);
    void reset() noexcept;

private:
    FundamentalCourseEngine engine_;
    std::size_t currentStep_{0};
};

} // namespace vll::curriculum
