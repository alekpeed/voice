#include "TestFramework.h"
#include "vll/curriculum/CurriculumCatalog.h"
#include "vll/curriculum/FundamentalCourse.h"
#include "vll/notation/SvgNotationRenderer.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {

std::vector<vll::Sonority> targetResponse(
    const vll::curriculum::FundamentalCourseStep& step) {
    std::vector<vll::Sonority> response;
    for (const auto& chord : step.chords) response.push_back(chord.target);
    return response;
}

} // namespace

TEST_CASE("fundamental course provides a fixed three-step two-voice path") {
    const auto& steps = vll::curriculum::FundamentalCourseEngine::steps();
    REQUIRE_EQ(steps.size(), std::size_t{3});
    REQUIRE_EQ(steps[0].id, std::string{"VL-03.2-2V-A"});
    REQUIRE_EQ(steps[1].id, std::string{"VL-03.1-2V-B"});
    REQUIRE_EQ(steps[2].id, std::string{"VL-05.2-2V-C"});
    REQUIRE_EQ(steps[0].chords.size(), std::size_t{2});
    REQUIRE_EQ(steps[1].chords.size(), std::size_t{2});
    REQUIRE_EQ(steps[2].chords.size(), std::size_t{3});
}

TEST_CASE("course steps reference only canonical concepts and printed exercises") {
    std::unordered_set<std::string_view> concepts;
    for (const auto& conceptReference : vll::curriculum::CurriculumCatalog::voiceLeadingConcepts()) {
        concepts.insert(conceptReference.id);
    }
    std::unordered_set<std::string_view> exercises;
    for (const auto& exercise : vll::curriculum::CurriculumCatalog::practiceExercises()) {
        exercises.insert(exercise.id);
    }

    for (const auto& step : vll::curriculum::FundamentalCourseEngine::steps()) {
        REQUIRE(!step.conceptIds.empty());
        for (const auto& conceptId : step.conceptIds) REQUIRE(concepts.contains(conceptId));
        REQUIRE(std::ranges::any_of(step.conceptIds, [&](const std::string& conceptId) {
            return step.id.starts_with(conceptId + '-');
        }));
        REQUIRE(exercises.contains(step.bookExerciseId));
    }
}

TEST_CASE("complete target is the exact guide-tone ii-V-I skeleton") {
    const auto& step = vll::curriculum::FundamentalCourseEngine::steps().back();
    REQUIRE_EQ(step.chords[0].symbol, std::string{"Dm7"});
    REQUIRE_EQ(step.chords[1].symbol, std::string{"G7"});
    REQUIRE_EQ(step.chords[2].symbol, std::string{"Cmaj7"});
    REQUIRE_EQ(step.chords[0].target.pitches,
               std::vector<vll::Pitch>({{53}, {60}}));
    REQUIRE_EQ(step.chords[1].target.pitches,
               std::vector<vll::Pitch>({{53}, {59}}));
    REQUIRE_EQ(step.chords[2].target.pitches,
               std::vector<vll::Pitch>({{52}, {59}}));
}

TEST_CASE("accepted course attempt produces feedback paths and notation") {
    const auto& step = vll::curriculum::FundamentalCourseEngine::steps().back();
    const auto result = vll::curriculum::FundamentalCourseEngine{}.evaluate(
        step, targetResponse(step));
    REQUIRE(result.accepted);
    REQUIRE_EQ(result.transitionFeedback.size(), std::size_t{2});
    REQUIRE_EQ(result.voicePaths.size(), std::size_t{2});
    REQUIRE_EQ(result.voicePaths[0].points.size(), std::size_t{3});
    REQUIRE_EQ(result.voicePaths[0].points[0].pitch, vll::Pitch{53});
    REQUIRE_EQ(result.voicePaths[0].points[1].pitch, vll::Pitch{53});
    REQUIRE_EQ(result.voicePaths[0].points[2].pitch, vll::Pitch{52});
    REQUIRE_EQ(result.voicePaths[1].points[0].pitch, vll::Pitch{60});
    REQUIRE_EQ(result.voicePaths[1].points[1].pitch, vll::Pitch{59});
    REQUIRE_EQ(result.voicePaths[1].points[2].pitch, vll::Pitch{59});
    REQUIRE_EQ(result.notation.events.size(), std::size_t{3});
}

TEST_CASE("course feedback states both guide-tone resolutions exactly") {
    const auto& step = vll::curriculum::FundamentalCourseEngine::steps().back();
    const auto result = vll::curriculum::FundamentalCourseEngine{}.evaluate(
        step, targetResponse(step));
    REQUIRE(std::ranges::any_of(result.observations, [](const vll::Observation& observation) {
        return observation.fact ==
            "Voice 2 moved from Dm7 guide tone C4 to G7 guide tone B3.";
    }));
    REQUIRE(std::ranges::any_of(result.observations, [](const vll::Observation& observation) {
        return observation.fact ==
            "Voice 1 moved from G7 guide tone F3 to Cmaj7 guide tone E3.";
    }));
}

TEST_CASE("course notation renders the accepted cadence compactly") {
    const auto& step = vll::curriculum::FundamentalCourseEngine::steps().back();
    const auto result = vll::curriculum::FundamentalCourseEngine{}.evaluate(
        step, targetResponse(step));
    const auto rendered = vll::notation::SvgNotationRenderer{}.render(result.notation, {});
    REQUIRE_EQ(rendered.systemCount, std::size_t{1});
    REQUIRE(rendered.width < 500.0F);
    REQUIRE(rendered.scalableVectorData.find("Dm7") != std::string::npos);
    REQUIRE(rendered.scalableVectorData.find("G7") != std::string::npos);
    REQUIRE(rendered.scalableVectorData.find("Cmaj7") != std::string::npos);
}

TEST_CASE("wrong sonority receives an exact factual mismatch and does not pass") {
    const auto& step = vll::curriculum::FundamentalCourseEngine::steps().front();
    auto response = targetResponse(step);
    response[1].pitches = {{53}, {60}};
    const auto result = vll::curriculum::FundamentalCourseEngine{}.evaluate(step, response);
    REQUIRE(!result.accepted);
    REQUIRE_EQ(result.observations.size(), std::size_t{1});
    REQUIRE_EQ(result.observations[0].code, std::string{"sonority_mismatch"});
    REQUIRE_EQ(result.observations[0].fact,
               std::string{"Event 2 (G7) expected F3, B3; received F3, C4."});
}

TEST_CASE("course rejects incomplete and nonmonotonic responses explicitly") {
    const auto& step = vll::curriculum::FundamentalCourseEngine::steps().front();
    auto incomplete = targetResponse(step);
    incomplete.pop_back();
    auto result = vll::curriculum::FundamentalCourseEngine{}.evaluate(step, incomplete);
    REQUIRE_EQ(result.observations[0].code, std::string{"sonority_count_mismatch"});

    auto nonmonotonic = targetResponse(step);
    nonmonotonic[1].startedAt = nonmonotonic[0].startedAt;
    result = vll::curriculum::FundamentalCourseEngine{}.evaluate(step, nonmonotonic);
    REQUIRE_EQ(result.observations[0].code, std::string{"nonmonotonic_response"});
}

TEST_CASE("pitch order does not change a correct course response") {
    const auto& step = vll::curriculum::FundamentalCourseEngine::steps().front();
    auto response = targetResponse(step);
    for (auto& event : response) std::ranges::reverse(event.pitches);
    REQUIRE(vll::curriculum::FundamentalCourseEngine{}.evaluate(step, response).accepted);
}

TEST_CASE("course session unlocks steps sequentially and completes") {
    vll::curriculum::FundamentalCourseSession session;
    REQUIRE_EQ(session.currentStepIndex(), std::size_t{0});
    REQUIRE(!session.complete());

    auto wrong = targetResponse(*session.currentStep());
    wrong[0].pitches.pop_back();
    REQUIRE(!session.submit(wrong).accepted);
    REQUIRE_EQ(session.currentStepIndex(), std::size_t{0});

    for (std::size_t index = 0; index < 3; ++index) {
        const auto response = targetResponse(*session.currentStep());
        const auto result = session.submit(response);
        REQUIRE(result.accepted);
        REQUIRE_EQ(result.courseComplete, index == 2);
    }
    REQUIRE(session.complete());
    REQUIRE(session.currentStep() == nullptr);

    const auto afterCompletion = session.submit({});
    REQUIRE_EQ(afterCompletion.observations[0].code,
               std::string{"course_already_complete"});
    session.reset();
    REQUIRE_EQ(session.currentStepIndex(), std::size_t{0});
}
