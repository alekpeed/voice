#include "vll/curriculum/FundamentalCourse.h"

#include "vll/analysis/VoiceAssigner.h"
#include "vll/visualization/VoicePathBuilder.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace vll::curriculum {
namespace {

Sonority chord(std::initializer_list<int> pitches, const TimestampMicros timestamp) {
    Sonority result;
    result.startedAt = timestamp;
    result.endedAt = timestamp + 80'000;
    for (const int pitch : pitches) result.pitches.push_back(Pitch{pitch});
    return result;
}

const std::vector<FundamentalCourseStep> kSteps{
    {
        "VL-03.2-2V-A",
        "Keep F; resolve C to B",
        "Play the two guide tones of Dm7, then keep F while C descends to B for G7.",
        {"VL-01.2", "VL-02.1", "VL-02.2", "VL-03.2", "VL-05.1"},
        "VL-EX-03C",
        {{"Dm7", {5, 0}, chord({53, 60}, 0)},
         {"G7", {11, 5}, chord({53, 59}, 1'000'000)}},
    },
    {
        "VL-03.1-2V-B",
        "Keep B; resolve F to E",
        "Play the two guide tones of G7, then keep B while F descends to E for Cmaj7.",
        {"VL-01.2", "VL-02.1", "VL-02.2", "VL-03.1", "VL-03.2", "VL-05.1"},
        "VL-EX-03C",
        {{"G7", {11, 5}, chord({53, 59}, 0)},
         {"Cmaj7", {4, 11}, chord({52, 59}, 1'000'000)}},
    },
    {
        "VL-05.2-2V-C",
        "Connect the complete ii-V-I skeleton",
        "Play F-C for Dm7, F-B for G7, and E-B for Cmaj7 as two continuous voices.",
        {"VL-01.2", "VL-01.3", "VL-02.1", "VL-02.2", "VL-03.1", "VL-03.2",
         "VL-05.1", "VL-05.2"},
        "VL-EX-05A",
        {{"Dm7", {5, 0}, chord({53, 60}, 0)},
         {"G7", {11, 5}, chord({53, 59}, 1'000'000)},
         {"Cmaj7", {4, 11}, chord({52, 59}, 2'000'000)}},
    },
};

std::vector<Pitch> normalizedPitches(const Sonority& sonority) {
    auto pitches = sonority.pitches;
    std::ranges::sort(pitches);
    const auto duplicate = std::ranges::unique(pitches);
    pitches.erase(duplicate.begin(), duplicate.end());
    return pitches;
}

std::string pitchName(const Pitch pitch) {
    static constexpr std::array<const char*, 12> names{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return std::string{names[static_cast<std::size_t>(pitch.pitchClass())]} +
           std::to_string(pitch.midiNote / 12 - 1);
}

std::string pitchList(const std::vector<Pitch>& pitches) {
    std::ostringstream output;
    for (std::size_t index = 0; index < pitches.size(); ++index) {
        if (index > 0) output << ", ";
        output << pitchName(pitches[index]);
    }
    return output.str();
}

bool timestampsIncrease(const std::vector<Sonority>& response) {
    for (std::size_t index = 1; index < response.size(); ++index) {
        if (response[index].startedAt <= response[index - 1].startedAt) return false;
    }
    return true;
}

notation::NotationDocument notationFor(
    const FundamentalCourseStep& step,
    const std::vector<VoicePath>& paths) {
    notation::NotationDocument document;
    document.events.reserve(step.chords.size());
    for (std::size_t eventIndex = 0; eventIndex < step.chords.size(); ++eventIndex) {
        notation::NotationEvent event;
        event.timestamp = step.chords[eventIndex].target.startedAt;
        event.chordSymbol = step.chords[eventIndex].symbol;
        event.durationBeats = eventIndex + 1 == step.chords.size() ? 2.0 : 1.0;
        for (const auto& path : paths) {
            if (eventIndex < path.points.size()) {
                event.notes.push_back({path.points[eventIndex].pitch, path.voice.id,
                                       std::nullopt, false});
            }
        }
        std::ranges::sort(event.notes, {}, &notation::NotationNote::voiceId);
        document.events.push_back(std::move(event));
    }
    if (!document.events.empty()) document.playbackCursor = document.events.front().timestamp;
    return document;
}

} // namespace

const std::vector<FundamentalCourseStep>& FundamentalCourseEngine::steps() { return kSteps; }

CourseAttemptResult FundamentalCourseEngine::evaluate(
    const FundamentalCourseStep& step,
    const std::vector<Sonority>& response) const {
    CourseAttemptResult result;
    if (response.size() != step.chords.size()) {
        std::ostringstream fact;
        fact << "Step " << step.id << " expects " << step.chords.size()
             << " sonorities; received " << response.size() << '.';
        result.observations.push_back({"sonority_count_mismatch", fact.str()});
        return result;
    }
    if (!timestampsIncrease(response)) {
        result.observations.push_back(
            {"nonmonotonic_response", "Response sonority timestamps must increase."});
        return result;
    }

    bool matches = true;
    for (std::size_t index = 0; index < step.chords.size(); ++index) {
        const auto expected = normalizedPitches(step.chords[index].target);
        const auto received = normalizedPitches(response[index]);
        if (expected == received) continue;
        matches = false;
        std::ostringstream fact;
        fact << "Event " << index + 1 << " (" << step.chords[index].symbol
             << ") expected " << pitchList(expected) << "; received "
             << (received.empty() ? std::string{"no pitches"} : pitchList(received)) << '.';
        result.observations.push_back({"sonority_mismatch", fact.str()});
    }
    if (!matches) return result;

    std::vector<Sonority> canonicalResponse = response;
    for (std::size_t index = 0; index < canonicalResponse.size(); ++index) {
        canonicalResponse[index].startedAt = step.chords[index].target.startedAt;
        canonicalResponse[index].endedAt = step.chords[index].target.endedAt;
    }
    ExerciseConstraint constraints;
    constraints.voiceCount = 2;
    constraints.maximumLeap = 12;
    const analysis::VoiceAssigner assigner;
    const auto paths = visualization::VoicePathBuilder(assigner).build(
        canonicalResponse, constraints);
    if (!paths.complete || paths.assignments.size() + 1 != step.chords.size()) {
        result.observations.push_back(
            {"voice_assignment_failed", "The two response voices could not be assigned across the progression."});
        return result;
    }

    const analysis::DeterministicFeedbackGenerator feedbackGenerator;
    for (std::size_t index = 0; index < paths.assignments.size(); ++index) {
        const analysis::FeedbackContext context{
            analysis::ChordFeedbackContext{step.chords[index].symbol,
                                           step.chords[index].guideTonePitchClasses},
            analysis::ChordFeedbackContext{step.chords[index + 1].symbol,
                                           step.chords[index + 1].guideTonePitchClasses},
            analysis::PitchNamePreference::Sharps,
        };
        auto feedback = feedbackGenerator.generate(paths.assignments[index], context);
        result.observations.insert(result.observations.end(),
                                   feedback.observations.begin(), feedback.observations.end());
        result.transitionFeedback.push_back(std::move(feedback));
    }
    result.voicePaths = paths.paths;
    result.notation = notationFor(step, result.voicePaths);
    result.accepted = true;
    return result;
}

std::size_t FundamentalCourseSession::currentStepIndex() const noexcept { return currentStep_; }

const FundamentalCourseStep* FundamentalCourseSession::currentStep() const noexcept {
    return complete() ? nullptr : &FundamentalCourseEngine::steps()[currentStep_];
}

bool FundamentalCourseSession::complete() const noexcept {
    return currentStep_ >= FundamentalCourseEngine::steps().size();
}

CourseAttemptResult FundamentalCourseSession::submit(const std::vector<Sonority>& response) {
    CourseAttemptResult result;
    if (complete()) {
        result.courseComplete = true;
        result.observations.push_back(
            {"course_already_complete", "The two-voice ii-V-I course path is complete."});
        return result;
    }
    result = engine_.evaluate(*currentStep(), response);
    if (result.accepted) ++currentStep_;
    result.courseComplete = complete();
    return result;
}

void FundamentalCourseSession::reset() noexcept { currentStep_ = 0; }

} // namespace vll::curriculum
