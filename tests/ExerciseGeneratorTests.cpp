#include "TestFramework.h"
#include "vll/exercise/DeterministicExerciseGenerator.h"
#include "vll/exercise/NearestVoicingSolver.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

vll::exercise::ExerciseGenerationRequest request(
    const int key, const int voices, const vll::exercise::ExerciseType type,
    const std::uint64_t seed = 1) {
    return {"VL-05.2-GEN", seed, key, voices, type};
}

} // namespace

TEST_CASE("nearest solver finds the canonical two-guide-tone resolution") {
    const vll::exercise::VoicingSearchRequest search{
        {{{53}, {60}}, 0, 80'000},
        {7, 11, 2, 5},
        {11, 5},
        2,
        41,
        72,
        12,
        19,
        std::nullopt,
        std::nullopt,
    };
    const auto solution = vll::exercise::NearestVoicingSolver{}.solve(search);
    REQUIRE(solution.found);
    REQUIRE_EQ(solution.destination.pitches,
               std::vector<vll::Pitch>({{53}, {59}}));
    REQUIRE_EQ(solution.metrics.totalSemitoneDisplacement, 1);
    REQUIRE_EQ(solution.metrics.stationaryVoices, 1);
    REQUIRE_EQ(solution.metrics.semitoneMoves, 1);
}

TEST_CASE("generator transposes the guide-tone exercise through all keys") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto reference = generator.generate(
        request(0, 2, vll::exercise::ExerciseType::GuideTonesOnly));
    REQUIRE(reference.valid);
    for (int key = 0; key < 12; ++key) {
        const auto prompt = generator.generate(
            request(key, 2, vll::exercise::ExerciseType::GuideTonesOnly));
        REQUIRE(prompt.valid);
        REQUIRE_EQ(prompt.keyPitchClass, key);
        REQUIRE_EQ(prompt.constraints.voiceCount, 2);
        REQUIRE_EQ(prompt.source.pitches.size(), std::size_t{2});
        REQUIRE_EQ(prompt.target.pitches.size(), std::size_t{2});
        for (std::size_t index = 0; index < 2; ++index) {
            REQUIRE_EQ(prompt.source.pitches[index].midiNote,
                       reference.source.pitches[index].midiNote + key);
            REQUIRE_EQ(prompt.target.pitches[index].midiNote,
                       reference.target.pitches[index].midiNote + key);
        }
    }
}

TEST_CASE("generator supports two three and four voice nearest voicings") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    for (int voices = 2; voices <= 4; ++voices) {
        const auto prompt = generator.generate(
            request(0, voices, vll::exercise::ExerciseType::NearestVoicing));
        REQUIRE(prompt.valid);
        REQUIRE_EQ(prompt.constraints.voiceCount, voices);
        REQUIRE_EQ(prompt.source.pitches.size(), static_cast<std::size_t>(voices));
        REQUIRE_EQ(prompt.target.pitches.size(), static_cast<std::size_t>(voices));
    }
}

TEST_CASE("identical seeds reproduce every generated prompt field") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto first = generator.generate("VL-05.2-NV", 983451);
    const auto second = generator.generate("VL-05.2-NV", 983451);
    REQUIRE(first.valid);
    REQUIRE_EQ(first.keyPitchClass, second.keyPitchClass);
    REQUIRE_EQ(first.constraints.voiceCount, second.constraints.voiceCount);
    REQUIRE_EQ(first.type, second.type);
    REQUIRE_EQ(first.source.pitches, second.source.pitches);
    REQUIRE_EQ(first.target.pitches, second.target.pitches);
    REQUIRE_EQ(first.optimalTotalDisplacement, second.optimalTotalDisplacement);
    REQUIRE_EQ(first.instruction, second.instruction);
}

TEST_CASE("different seeds vary default key or voice count") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto reference = generator.generate("VL-05.2-NV", 0);
    bool varied = false;
    for (std::uint64_t seed = 1; seed < 16; ++seed) {
        const auto prompt = generator.generate("VL-05.2-NV", seed);
        if (prompt.keyPitchClass != reference.keyPitchClass ||
            prompt.constraints.voiceCount != reference.constraints.voiceCount) {
            varied = true;
        }
    }
    REQUIRE(varied);
}

TEST_CASE("fixed soprano and fixed bass prompts expose hard target locks") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto soprano = generator.generate(
        request(3, 4, vll::exercise::ExerciseType::FixedSoprano));
    REQUIRE(soprano.valid);
    REQUIRE(soprano.constraints.lockedSoprano.has_value());
    REQUIRE_EQ(*soprano.constraints.lockedSoprano, soprano.target.pitches.back());
    REQUIRE(generator.submit(soprano, soprano.target).satisfiesConstraints);

    const auto bass = generator.generate(
        request(8, 3, vll::exercise::ExerciseType::FixedBass));
    REQUIRE(bass.valid);
    REQUIRE(bass.constraints.lockedBass.has_value());
    REQUIRE_EQ(*bass.constraints.lockedBass, bass.target.pitches.front());
    REQUIRE(generator.submit(bass, bass.target).satisfiesConstraints);
}

TEST_CASE("guide-tone-only mode forces two voices and both required tones") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto prompt = generator.generate(
        request(0, 4, vll::exercise::ExerciseType::GuideTonesOnly));
    REQUIRE(prompt.valid);
    REQUIRE_EQ(prompt.constraints.voiceCount, 2);
    REQUIRE_EQ(prompt.requiredDestinationPitchClasses,
               std::vector<int>({11, 5}));
    REQUIRE_EQ(prompt.target.pitches,
               std::vector<vll::Pitch>({{53}, {59}}));
}

TEST_CASE("generated nearest target is accepted deterministically") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto prompt = generator.generate(
        request(0, 4, vll::exercise::ExerciseType::NearestVoicing));
    const auto result = generator.submit(prompt, prompt.target);
    REQUIRE(result.satisfiesConstraints);
    REQUIRE_EQ(result.observations.size(), std::size_t{1});
    REQUIRE_EQ(result.observations[0].code, std::string{"nearest_voicing_matched"});
}

TEST_CASE("valid but farther voicing is rejected with exact displacement") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto prompt = generator.generate(
        request(0, 2, vll::exercise::ExerciseType::GuideTonesOnly));
    vll::Sonority farther{{{53}, {71}}, 1'000'000, 1'080'000};
    const auto result = generator.submit(prompt, farther);
    REQUIRE(!result.satisfiesConstraints);
    REQUIRE_EQ(result.observations[0].code, std::string{"not_nearest_voicing"});
    REQUIRE_EQ(result.observations[0].fact,
               std::string{"The response moves 11 total semitones; the nearest valid voicing moves 1."});
}

TEST_CASE("submission enforces chord membership required tones and outer locks") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    auto prompt = generator.generate(
        request(0, 3, vll::exercise::ExerciseType::NearestVoicing));
    auto result = generator.submit(prompt, {{{53}, {59}, {61}}, 0, 0});
    REQUIRE_EQ(result.observations[0].code, std::string{"non_chord_tone"});

    result = generator.submit(prompt, {{{50}, {55}, {59}}, 0, 0});
    REQUIRE_EQ(result.observations[0].code, std::string{"required_tone_missing"});

    prompt = generator.generate(
        request(0, 3, vll::exercise::ExerciseType::FixedSoprano));
    auto wrongLock = prompt.target;
    wrongLock.pitches.back().midiNote -= 12;
    result = generator.submit(prompt, wrongLock);
    REQUIRE_EQ(result.observations[0].code, std::string{"soprano_lock_failed"});
}

TEST_CASE("solver satisfies explicit bass and soprano locks") {
    const vll::exercise::VoicingSearchRequest search{
        {{{48}, {52}, {55}, {59}}, 0, 80'000},
        {5, 9, 0, 4},
        {5, 9, 0, 4},
        4,
        36,
        72,
        12,
        19,
        vll::Pitch{41},
        vll::Pitch{64},
    };
    const auto solution = vll::exercise::NearestVoicingSolver{}.solve(search);
    REQUIRE(solution.found);
    REQUIRE_EQ(solution.destination.pitches.front(), vll::Pitch{41});
    REQUIRE_EQ(solution.destination.pitches.back(), vll::Pitch{64});
}

TEST_CASE("transposition preserves generated source and destination intervals") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    const auto c = generator.generate(
        request(0, 4, vll::exercise::ExerciseType::NearestVoicing));
    const auto f = generator.generate(
        request(5, 4, vll::exercise::ExerciseType::NearestVoicing));
    REQUIRE(c.valid);
    REQUIRE(f.valid);
    for (std::size_t index = 0; index < c.source.pitches.size(); ++index) {
        REQUIRE_EQ(f.source.pitches[index].midiNote, c.source.pitches[index].midiNote + 5);
        REQUIRE_EQ(f.target.pitches[index].midiNote, c.target.pitches[index].midiNote + 5);
    }
    REQUIRE_EQ(c.optimalTotalDisplacement, f.optimalTotalDisplacement);
}

TEST_CASE("invalid IDs and unsupported voice counts fail explicitly") {
    const vll::exercise::DeterministicExerciseGenerator generator;
    auto prompt = generator.generate("not-canonical", 1);
    REQUIRE(!prompt.valid);
    REQUIRE_EQ(generator.submit(prompt, {}).observations[0].code,
               std::string{"invalid_prompt"});

    prompt = generator.generate(request(0, 5, vll::exercise::ExerciseType::NearestVoicing));
    REQUIRE(!prompt.valid);
    REQUIRE(prompt.error.find("two, three, or four") != std::string::npos);
}
