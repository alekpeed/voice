#include "vll/exercise/DeterministicExerciseGenerator.h"

#include "vll/analysis/VoiceAssigner.h"
#include "vll/analysis/VoiceLeadingEvaluator.h"
#include "vll/exercise/NearestVoicingSolver.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace vll::exercise {
namespace {

class StableRandom final {
public:
    explicit StableRandom(const std::uint64_t seed) : state_(seed) {}

    std::uint64_t next() noexcept {
        state_ += 0x9e3779b97f4a7c15ULL;
        std::uint64_t value = state_;
        value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31U);
    }

private:
    std::uint64_t state_{0};
};

int normalizedPitchClass(const int pitchClass) {
    const int value = pitchClass % 12;
    return value < 0 ? value + 12 : value;
}

std::string pitchClassName(const int pitchClass) {
    static constexpr std::array<std::string_view, 12> names{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return std::string{names[static_cast<std::size_t>(normalizedPitchClass(pitchClass))]};
}

ExerciseType typeFromId(const std::string& exerciseId) {
    if (exerciseId.ends_with("-GT")) return ExerciseType::GuideTonesOnly;
    if (exerciseId.ends_with("-FS")) return ExerciseType::FixedSoprano;
    if (exerciseId.ends_with("-FB")) return ExerciseType::FixedBass;
    return ExerciseType::NearestVoicing;
}

std::string typeInstruction(const ExerciseType type) {
    switch (type) {
        case ExerciseType::NearestVoicing:
            return "Move to the nearest valid dominant voicing.";
        case ExerciseType::FixedSoprano:
            return "Keep the displayed soprano fixed and complete the nearest dominant voicing.";
        case ExerciseType::FixedBass:
            return "Keep the displayed bass fixed and complete the nearest dominant voicing.";
        case ExerciseType::GuideTonesOnly:
            return "Move only the third and seventh from ii7 to V7 by the nearest paths.";
    }
    return {};
}

std::vector<Pitch> sourceTemplate(const int voiceCount) {
    if (voiceCount == 2) return {{53}, {60}};
    if (voiceCount == 3) return {{50}, {53}, {60}};
    return {{50}, {53}, {57}, {60}};
}

std::vector<Pitch> transposed(std::vector<Pitch> pitches, const int semitones) {
    for (auto& pitch : pitches) pitch.midiNote += semitones;
    return pitches;
}

std::vector<Pitch> normalizedPitches(const Sonority& sonority) {
    auto pitches = sonority.pitches;
    std::ranges::sort(pitches);
    const auto duplicate = std::ranges::unique(pitches);
    pitches.erase(duplicate.begin(), duplicate.end());
    return pitches;
}

bool containsPitchClass(const std::vector<int>& pitchClasses, const Pitch pitch) {
    return std::ranges::any_of(pitchClasses, [&](const int pitchClass) {
        return normalizedPitchClass(pitchClass) == pitch.pitchClass();
    });
}

bool containsPitchClass(const std::vector<Pitch>& pitches, const int pitchClass) {
    return std::ranges::any_of(pitches, [&](const Pitch pitch) {
        return pitch.pitchClass() == normalizedPitchClass(pitchClass);
    });
}

std::string pitchName(const Pitch pitch) {
    return pitchClassName(pitch.pitchClass()) + std::to_string(pitch.midiNote / 12 - 1);
}

} // namespace

ExercisePrompt DeterministicExerciseGenerator::generate(
    const std::string& exerciseId, const std::uint64_t seed) const {
    return generate({exerciseId, seed, std::nullopt, std::nullopt, std::nullopt});
}

ExercisePrompt DeterministicExerciseGenerator::generate(
    const ExerciseGenerationRequest& request) const {
    ExercisePrompt prompt;
    prompt.id = request.exerciseId;
    prompt.seed = request.seed;
    if (request.exerciseId.empty() || !request.exerciseId.starts_with("VL-")) {
        prompt.error = "Exercise IDs must reference a canonical VL concept.";
        return prompt;
    }

    StableRandom random(request.seed);
    prompt.keyPitchClass = normalizedPitchClass(
        request.keyPitchClass.value_or(static_cast<int>(random.next() % 12ULL)));
    prompt.type = request.type.value_or(typeFromId(request.exerciseId));
    int voiceCount = request.voiceCount.value_or(2 + static_cast<int>(random.next() % 3ULL));
    if (prompt.type == ExerciseType::GuideTonesOnly) voiceCount = 2;
    if (voiceCount < 2 || voiceCount > 4) {
        prompt.error = "Generated exercises support two, three, or four voices.";
        return prompt;
    }

    prompt.constraints.voiceCount = voiceCount;
    prompt.constraints.maximumLeap = 12;
    prompt.sourceChord = pitchClassName(prompt.keyPitchClass + 2) + "m7";
    prompt.destinationChord = pitchClassName(prompt.keyPitchClass + 7) + "7";
    prompt.instruction = typeInstruction(prompt.type) + " Key: " +
                         pitchClassName(prompt.keyPitchClass) + " major.";
    prompt.source = {transposed(sourceTemplate(voiceCount), prompt.keyPitchClass), 0, 80'000};
    prompt.allowedDestinationPitchClasses = {
        normalizedPitchClass(prompt.keyPitchClass + 7),
        normalizedPitchClass(prompt.keyPitchClass + 11),
        normalizedPitchClass(prompt.keyPitchClass + 2),
        normalizedPitchClass(prompt.keyPitchClass + 5),
    };
    prompt.requiredDestinationPitchClasses = {
        normalizedPitchClass(prompt.keyPitchClass + 11),
        normalizedPitchClass(prompt.keyPitchClass + 5),
    };
    if (voiceCount == 4) {
        prompt.requiredDestinationPitchClasses = prompt.allowedDestinationPitchClasses;
    }
    prompt.minimumMidi = std::max(24, prompt.source.pitches.front().midiNote - 12);
    prompt.maximumMidi = std::min(96, prompt.source.pitches.back().midiNote + 12);

    VoicingSearchRequest search{
        prompt.source,
        prompt.allowedDestinationPitchClasses,
        prompt.requiredDestinationPitchClasses,
        voiceCount,
        prompt.minimumMidi,
        prompt.maximumMidi,
        prompt.constraints.maximumLeap,
        prompt.maximumAdjacentSpacing,
        std::nullopt,
        std::nullopt,
    };
    const NearestVoicingSolver solver;
    auto solution = solver.solve(search);
    if (!solution.found) {
        prompt.error = "No valid destination voicing exists in the generated register.";
        return prompt;
    }
    if (prompt.type == ExerciseType::FixedBass) {
        prompt.constraints.lockedBass = solution.destination.pitches.front();
        search.lockedBass = prompt.constraints.lockedBass;
        solution = solver.solve(search);
    } else if (prompt.type == ExerciseType::FixedSoprano) {
        prompt.constraints.lockedSoprano = solution.destination.pitches.back();
        search.lockedSoprano = prompt.constraints.lockedSoprano;
        solution = solver.solve(search);
    }
    if (!solution.found) {
        prompt.error = "The generated outer-voice lock has no valid solution.";
        return prompt;
    }

    prompt.target = solution.destination;
    prompt.optimalTotalDisplacement = solution.metrics.totalSemitoneDisplacement;
    prompt.valid = true;
    return prompt;
}

AttemptResult DeterministicExerciseGenerator::submit(
    const ExercisePrompt& prompt, const Sonority& response) const {
    AttemptResult result;
    if (!prompt.valid) {
        result.observations.push_back({"invalid_prompt", prompt.error.empty()
            ? "The exercise prompt is invalid." : prompt.error});
        return result;
    }

    const auto pitches = normalizedPitches(response);
    if (pitches.size() != response.pitches.size()) {
        result.observations.push_back(
            {"duplicate_pitch", "The response contains a duplicate pitch."});
        return result;
    }
    if (pitches.size() != static_cast<std::size_t>(prompt.constraints.voiceCount)) {
        std::ostringstream fact;
        fact << "The response requires " << prompt.constraints.voiceCount
             << " voices; received " << pitches.size() << '.';
        result.observations.push_back({"voice_count_mismatch", fact.str()});
        return result;
    }
    for (const Pitch pitch : pitches) {
        if (pitch.midiNote < prompt.minimumMidi || pitch.midiNote > prompt.maximumMidi) {
            result.observations.push_back(
                {"register_out_of_range", pitchName(pitch) + " is outside the exercise register."});
            return result;
        }
        if (!containsPitchClass(prompt.allowedDestinationPitchClasses, pitch)) {
            result.observations.push_back(
                {"non_chord_tone", pitchName(pitch) + " is not a tone of " +
                 prompt.destinationChord + '.'});
            return result;
        }
    }
    for (const int pitchClass : prompt.requiredDestinationPitchClasses) {
        if (!containsPitchClass(pitches, pitchClass)) {
            result.observations.push_back(
                {"required_tone_missing", pitchClassName(pitchClass) +
                 " is required in the destination voicing."});
            return result;
        }
    }
    if (prompt.constraints.lockedBass && pitches.front() != *prompt.constraints.lockedBass) {
        result.observations.push_back(
            {"bass_lock_failed", "The destination bass must remain " +
             pitchName(*prompt.constraints.lockedBass) + '.'});
        return result;
    }
    if (prompt.constraints.lockedSoprano && pitches.back() != *prompt.constraints.lockedSoprano) {
        result.observations.push_back(
            {"soprano_lock_failed", "The destination soprano must remain " +
             pitchName(*prompt.constraints.lockedSoprano) + '.'});
        return result;
    }

    const Sonority destination{pitches, 1'000'000, 1'080'000};
    const analysis::VoiceAssigner assigner;
    const auto assignment = assigner.assign(prompt.source, destination, prompt.constraints);
    if (assignment.transitions.size() != pitches.size()) {
        result.observations.push_back(
            {"voice_assignment_failed", "The response voices could not be assigned."});
        return result;
    }
    const analysis::VoiceLeadingEvaluator evaluator;
    const auto metrics = evaluator.metrics(assignment);
    if (metrics.hasCrossing) {
        result.observations.push_back(
            {"voice_crossing", "The response crosses two assigned voices."});
        return result;
    }
    if (metrics.maximumAdjacentSpacing > prompt.maximumAdjacentSpacing) {
        result.observations.push_back(
            {"spacing_exceeded", "The response exceeds the maximum adjacent-voice spacing."});
        return result;
    }
    if (metrics.maximumVoiceMovement > prompt.constraints.maximumLeap) {
        result.observations.push_back(
            {"maximum_leap_exceeded", "The response exceeds the exercise movement limit."});
        return result;
    }
    if (metrics.totalSemitoneDisplacement != prompt.optimalTotalDisplacement) {
        std::ostringstream fact;
        fact << "The response moves " << metrics.totalSemitoneDisplacement
             << " total semitones; the nearest valid voicing moves "
             << prompt.optimalTotalDisplacement << '.';
        result.observations.push_back({"not_nearest_voicing", fact.str()});
        return result;
    }

    result.satisfiesConstraints = true;
    std::ostringstream fact;
    fact << "The response matches the nearest valid voicing at "
         << metrics.totalSemitoneDisplacement << " total semitones of movement.";
    result.observations.push_back({"nearest_voicing_matched", fact.str()});
    return result;
}

} // namespace vll::exercise
