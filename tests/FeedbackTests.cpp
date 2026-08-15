#include "TestFramework.h"
#include "vll/analysis/DeterministicFeedback.h"

#include <algorithm>
#include <cstddef>
#include <string>

namespace {

vll::analysis::VoiceAssignment iiToV() {
    return {
        {{1, {50}, {43}, -7, vll::MovementSize::Leap},
         {2, {53}, {53}, 0, vll::MovementSize::Stationary},
         {3, {60}, {59}, -1, vll::MovementSize::Semitone}},
        0.9,
        false,
    };
}

vll::analysis::FeedbackContext iiToVContext() {
    return {
        vll::analysis::ChordFeedbackContext{"Dm7", {5, 0}},
        vll::analysis::ChordFeedbackContext{"G7", {11, 5}},
        vll::analysis::PitchNamePreference::Sharps,
    };
}

} // namespace

TEST_CASE("feedback summary reports exact common step and leap counts") {
    const auto report = vll::analysis::DeterministicFeedbackGenerator{}.generate(
        iiToV(), iiToVContext());
    REQUIRE_EQ(report.observations.front().code, std::string{"transition_summary"});
    REQUIRE_EQ(report.observations.front().fact,
               std::string{"From Dm7 to G7: 1 common tone, 1 stepwise move, 1 leap; "
                           "total displacement 8 semitones."});
    REQUIRE_EQ(report.metrics.stationaryVoices, 1);
    REQUIRE_EQ(report.metrics.semitoneMoves, 1);
    REQUIRE_EQ(report.metrics.leaps, 1);
}

TEST_CASE("feedback states exact movement for every voice") {
    const auto report = vll::analysis::DeterministicFeedbackGenerator{}.generate(
        iiToV(), iiToVContext());
    REQUIRE(std::ranges::any_of(report.observations, [](const vll::Observation& observation) {
        return observation.code == "leap" &&
               observation.fact == "Voice 1 moved from D3 to G2: descending 7 semitones (a leap).";
    }));
    REQUIRE(std::ranges::any_of(report.observations, [](const vll::Observation& observation) {
        return observation.code == "common_tone" &&
               observation.fact == "Voice 2 retained F3 as a common tone.";
    }));
    REQUIRE(std::ranges::any_of(report.observations, [](const vll::Observation& observation) {
        return observation.code == "semitone_move" &&
               observation.fact == "Voice 3 moved from C4 to B3: descending 1 semitone.";
    }));
}

TEST_CASE("guide-tone facts use only explicit chord context") {
    const auto report = vll::analysis::DeterministicFeedbackGenerator{}.generate(
        iiToV(), iiToVContext());
    REQUIRE(std::ranges::any_of(report.observations, [](const vll::Observation& observation) {
        return observation.code == "guide_tone_connection" &&
               observation.fact ==
                   "Voice 2 retained F3 as a guide tone in both Dm7 and G7.";
    }));
    REQUIRE(std::ranges::any_of(report.observations, [](const vll::Observation& observation) {
        return observation.code == "guide_tone_connection" &&
               observation.fact ==
                   "Voice 3 moved from Dm7 guide tone C4 to G7 guide tone B3.";
    }));

    const auto withoutContext = vll::analysis::DeterministicFeedbackGenerator{}.generate(iiToV());
    REQUIRE(!std::ranges::any_of(withoutContext.observations, [](const vll::Observation& observation) {
        return observation.code.starts_with("guide_tone");
    }));
}

TEST_CASE("feedback distinguishes whole steps and ascending direction") {
    const vll::analysis::VoiceAssignment assignment{
        {{1, {48}, {50}, 2, vll::MovementSize::WholeStep},
         {2, {60}, {64}, 4, vll::MovementSize::Leap}},
        1.0,
        false,
    };
    const auto report = vll::analysis::DeterministicFeedbackGenerator{}.generate(assignment);
    REQUIRE_EQ(report.observations[1].code, std::string{"whole_step_move"});
    REQUIRE_EQ(report.observations[1].fact,
               std::string{"Voice 1 moved from C3 to D3: ascending 2 semitones (a whole step)."});
    REQUIRE_EQ(report.observations[2].fact,
               std::string{"Voice 2 moved from C4 to E4: ascending 4 semitones (a leap)."});
}

TEST_CASE("crossing feedback identifies exact voices and destination pitches") {
    const vll::analysis::VoiceAssignment assignment{
        {{1, {60}, {67}, 7, vll::MovementSize::Leap},
         {2, {64}, {60}, -4, vll::MovementSize::Leap}},
        0.7,
        false,
    };
    const auto report = vll::analysis::DeterministicFeedbackGenerator{}.generate(assignment);
    REQUIRE(report.metrics.hasCrossing);
    REQUIRE_EQ(report.observations.back().code, std::string{"voice_crossing"});
    REQUIRE_EQ(report.observations.back().fact,
               std::string{"Voices 1 and 2 crossed: voice 1 ended on G4 above voice 2 on C4."});
}

TEST_CASE("flat pitch names are available for contextual spelling") {
    const vll::analysis::VoiceAssignment assignment{
        {{1, {61}, {63}, 2, vll::MovementSize::WholeStep},
         {2, {68}, {68}, 0, vll::MovementSize::Stationary}},
        1.0,
        false,
    };
    vll::analysis::FeedbackContext context;
    context.pitchNames = vll::analysis::PitchNamePreference::Flats;
    const auto report = vll::analysis::DeterministicFeedbackGenerator{}.generate(assignment, context);
    REQUIRE(report.observations[1].fact.find("Db4 to Eb4") != std::string::npos);
    REQUIRE(report.observations[2].fact.find("Ab4") != std::string::npos);
}

TEST_CASE("feedback ordering is stable when assignment transitions are unordered") {
    auto ordered = iiToV();
    auto shuffled = ordered;
    std::ranges::reverse(shuffled.transitions);
    const auto first = vll::analysis::DeterministicFeedbackGenerator{}.generate(
        ordered, iiToVContext());
    const auto second = vll::analysis::DeterministicFeedbackGenerator{}.generate(
        shuffled, iiToVContext());
    REQUIRE_EQ(first.observations.size(), second.observations.size());
    for (std::size_t index = 0; index < first.observations.size(); ++index) {
        REQUIRE_EQ(first.observations[index].code, second.observations[index].code);
        REQUIRE_EQ(first.observations[index].fact, second.observations[index].fact);
    }
}

TEST_CASE("empty and ambiguous assignments are stated without invented analysis") {
    const auto empty = vll::analysis::DeterministicFeedbackGenerator{}.generate({});
    REQUIRE_EQ(empty.observations.size(), std::size_t{1});
    REQUIRE_EQ(empty.observations[0].code, std::string{"no_voice_transitions"});

    auto ambiguous = iiToV();
    ambiguous.ambiguous = true;
    const auto report = vll::analysis::DeterministicFeedbackGenerator{}.generate(ambiguous);
    REQUIRE_EQ(report.observations.back().code, std::string{"ambiguous_assignment"});
}
