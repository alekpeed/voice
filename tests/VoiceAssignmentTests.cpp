#include "TestFramework.h"
#include "vll/analysis/VoiceAssigner.h"
#include "vll/analysis/VoiceLeadingEvaluator.h"

#include <cstddef>
#include <string>

namespace {

vll::Sonority sonority(std::initializer_list<int> midiNotes) {
    vll::Sonority result;
    for (const int midiNote : midiNotes) result.pitches.push_back(vll::Pitch{midiNote});
    return result;
}

vll::ExerciseConstraint voices(const int count, const int maximumLeap = 12) {
    vll::ExerciseConstraint result;
    result.voiceCount = count;
    result.maximumLeap = maximumLeap;
    return result;
}

} // namespace

TEST_CASE("two-voice assignment retains a common tone and semitone path") {
    const vll::analysis::VoiceAssigner assigner;
    const auto assignment = assigner.assign(sonority({60, 67}), sonority({59, 67}), voices(2));
    REQUIRE_EQ(assignment.transitions.size(), std::size_t{2});
    REQUIRE(!assignment.ambiguous);
    REQUIRE(assignment.confidence > 0.8);
    REQUIRE_EQ(assignment.transitions[0].semitones, -1);
    REQUIRE_EQ(assignment.transitions[1].semitones, 0);
}

TEST_CASE("three-voice assignment follows register and common-tone continuity") {
    const vll::analysis::VoiceAssigner assigner;
    const auto assignment = assigner.assign(
        sonority({50, 60, 65}), sonority({55, 59, 65}), voices(3));
    REQUIRE_EQ(assignment.transitions.size(), std::size_t{3});
    REQUIRE_EQ(assignment.transitions[0].semitones, 5);
    REQUIRE_EQ(assignment.transitions[1].semitones, -1);
    REQUIRE_EQ(assignment.transitions[2].semitones, 0);
}

TEST_CASE("four-voice assignment returns bass-through-soprano voice identities") {
    const vll::analysis::VoiceAssigner assigner;
    const auto assignment = assigner.assign(
        sonority({48, 55, 59, 64}), sonority({45, 55, 60, 64}), voices(4));
    REQUIRE_EQ(assignment.transitions.size(), std::size_t{4});
    REQUIRE_EQ(assignment.transitions[0].voiceId, vll::VoiceId{1});
    REQUIRE_EQ(assignment.transitions[3].voiceId, vll::VoiceId{4});
    REQUIRE_EQ(assignment.transitions[0].semitones, -3);
    REQUIRE_EQ(assignment.transitions[1].semitones, 0);
    REQUIRE_EQ(assignment.transitions[2].semitones, 1);
    REQUIRE_EQ(assignment.transitions[3].semitones, 0);
}

TEST_CASE("transposition preserves voice-assignment movement") {
    const vll::analysis::VoiceAssigner assigner;
    const auto original = assigner.assign(
        sonority({48, 55, 59, 64}), sonority({45, 55, 60, 64}), voices(4));
    const auto transposed = assigner.assign(
        sonority({53, 60, 64, 69}), sonority({50, 60, 65, 69}), voices(4));
    REQUIRE_EQ(original.transitions.size(), transposed.transitions.size());
    REQUIRE_EQ(original.ambiguous, transposed.ambiguous);
    for (std::size_t index = 0; index < original.transitions.size(); ++index) {
        REQUIRE_EQ(original.transitions[index].semitones, transposed.transitions[index].semitones);
    }
}

TEST_CASE("locked outer voices are hard assignment constraints") {
    const vll::analysis::VoiceAssigner assigner;
    auto constraints = voices(3);
    constraints.lockedBass = vll::Pitch{55};
    constraints.lockedSoprano = vll::Pitch{67};
    const auto valid = assigner.assign(sonority({53, 60, 65}), sonority({55, 59, 67}), constraints);
    REQUIRE_EQ(valid.transitions.size(), std::size_t{3});
    REQUIRE_EQ(valid.transitions.front().to, vll::Pitch{55});
    REQUIRE_EQ(valid.transitions.back().to, vll::Pitch{67});

    constraints.lockedSoprano = vll::Pitch{66};
    const auto invalid = assigner.assign(sonority({53, 60, 65}), sonority({55, 59, 67}), constraints);
    REQUIRE(invalid.transitions.empty());
    REQUIRE(invalid.ambiguous);
    REQUIRE_EQ(invalid.confidence, 0.0);
}

TEST_CASE("equal-cost mappings report ambiguity") {
    const vll::analysis::VoiceAssigner assigner({0.0, 0.0, 0.0, 0.0, 0.0});
    const auto assignment = assigner.assign(sonority({60, 67}), sonority({62, 65}), voices(2));
    REQUIRE_EQ(assignment.transitions.size(), std::size_t{2});
    REQUIRE(assignment.ambiguous);
    REQUIRE(assignment.confidence <= 0.5);
}

TEST_CASE("transition metrics expose motion crossing overlap spacing and contours") {
    const vll::analysis::VoiceLeadingEvaluator evaluator;
    const vll::analysis::VoiceAssignment assignment{
        {{1, {60}, {67}, 7, vll::MovementSize::Leap},
         {2, {64}, {60}, -4, vll::MovementSize::Leap}},
        0.4,
        false};
    const auto metrics = evaluator.metrics(assignment);
    REQUIRE_EQ(metrics.totalSemitoneDisplacement, 11);
    REQUIRE_EQ(metrics.maximumVoiceMovement, 7);
    REQUIRE_EQ(metrics.leaps, 2);
    REQUIRE(metrics.hasCrossing);
    REQUIRE(metrics.hasOverlap);
    REQUIRE_EQ(metrics.maximumAdjacentSpacing, 7);
    REQUIRE_EQ(metrics.bassMovement, 7);
    REQUIRE_EQ(metrics.sopranoMovement, -4);
}

TEST_CASE("constraint evaluation reports exact excessive movement") {
    const vll::analysis::VoiceLeadingEvaluator evaluator;
    const vll::analysis::VoiceAssignment assignment{
        {{1, {48}, {62}, 14, vll::MovementSize::Leap},
         {2, {60}, {61}, 1, vll::MovementSize::Semitone}},
        0.8,
        false};
    const auto result = evaluator.evaluate(assignment, voices(2, 12));
    REQUIRE(!result.satisfiesConstraints);
    REQUIRE_EQ(result.observations.size(), std::size_t{1});
    REQUIRE_EQ(result.observations[0].code, std::string{"maximum_leap_exceeded"});
    REQUIRE(result.observations[0].fact.find("14 semitones") != std::string::npos);
}

TEST_CASE("assignment rejects unsupported or mismatched voice counts") {
    const vll::analysis::VoiceAssigner assigner;
    REQUIRE(assigner.assign(sonority({60}), sonority({62}), voices(1)).transitions.empty());
    REQUIRE(assigner.assign(sonority({60, 64}), sonority({59, 62, 67}), voices(3)).transitions.empty());
    REQUIRE(assigner.assign(sonority({48, 52, 55, 59, 64}),
                            sonority({47, 50, 55, 59, 64}), voices(4)).transitions.empty());
}
