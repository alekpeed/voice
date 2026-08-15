#include "vll/harmony/MelodyHarmonizer.h"

#include "vll/exercise/NearestVoicingSolver.h"
#include "vll/analysis/VoiceAssigner.h"
#include "vll/visualization/VoicePathBuilder.h"

#include <algorithm>
#include <limits>

namespace vll::harmony {
namespace {

Sonority startingVoicing(const MelodyHarmonyEvent& event, const int voices) {
    Sonority result;
    const int top = event.melody.midiNote;
    for (int note = top - 1; note >= 36 && static_cast<int>(result.pitches.size()) < voices - 1; --note) {
        if (std::ranges::find(event.chordPitchClasses, Pitch{note}.pitchClass()) !=
            event.chordPitchClasses.end()) result.pitches.push_back(Pitch{note});
    }
    std::ranges::sort(result.pitches);
    result.pitches.push_back(event.melody);
    return result;
}

} // namespace

HarmonizationResult MelodyHarmonizer::harmonize(const HarmonizationRequest& request) const {
    HarmonizationResult result;
    if (request.events.empty() || request.voiceCount < 2 || request.voiceCount > 4 ||
        request.candidatesPerEvent == 0) {
        result.error = "Harmonization requires events, 2-4 voices, and at least one candidate.";
        return result;
    }
    Sonority previous = request.initialVoicing.value_or(
        startingVoicing(request.events.front(), request.voiceCount));
    if (static_cast<int>(previous.pitches.size()) != request.voiceCount) {
        result.error = "Initial voicing does not match the requested voice count.";
        return result;
    }
    const exercise::NearestVoicingSolver solver;
    std::vector<Sonority> selected;
    for (std::size_t eventIndex = 0; eventIndex < request.events.size(); ++eventIndex) {
        const auto& event = request.events[eventIndex];
        if (std::ranges::find(event.chordPitchClasses, event.melody.pitchClass()) ==
            event.chordPitchClasses.end()) {
            result.error = "Melody pitch is not present in the supplied harmony.";
            return result;
        }
        HarmonizedEvent harmonized{event, {}, 0};
        for (int bass = request.minimumMidi; bass <= event.melody.midiNote; ++bass) {
            if (std::ranges::find(event.chordPitchClasses, Pitch{bass}.pitchClass()) ==
                event.chordPitchClasses.end()) continue;
            const auto solution = solver.solve({
                previous, event.chordPitchClasses, {}, request.voiceCount,
                request.minimumMidi, std::min(request.maximumMidi, event.melody.midiNote),
                12, 19, Pitch{bass}, event.melody});
            if (!solution.found) continue;
            if (std::ranges::any_of(harmonized.candidates, [&](const auto& candidate) {
                return candidate.voicing.pitches == solution.destination.pitches;
            })) continue;
            harmonized.candidates.push_back({solution.destination,
                                             solution.metrics.totalSemitoneDisplacement,
                                             solution.metrics.maximumVoiceMovement});
        }
        std::ranges::sort(harmonized.candidates, [](const auto& left, const auto& right) {
            if (left.totalDisplacement != right.totalDisplacement)
                return left.totalDisplacement < right.totalDisplacement;
            if (left.maximumMovement != right.maximumMovement)
                return left.maximumMovement < right.maximumMovement;
            return left.voicing.pitches < right.voicing.pitches;
        });
        if (harmonized.candidates.empty()) {
            result.error = "No voicing satisfies the melody and harmony constraints.";
            return result;
        }
        if (harmonized.candidates.size() > request.candidatesPerEvent)
            harmonized.candidates.resize(request.candidatesPerEvent);
        previous = harmonized.candidates.front().voicing;
        previous.startedAt = static_cast<TimestampMicros>(eventIndex) * 1'000'000;
        previous.endedAt = previous.startedAt + 800'000;
        selected.push_back(previous);
        result.events.push_back(std::move(harmonized));
    }
    ExerciseConstraint constraints;
    constraints.voiceCount = request.voiceCount;
    constraints.maximumLeap = 12;
    analysis::VoiceAssigner assigner;
    const auto paths = visualization::VoicePathBuilder(assigner).build(selected, constraints);
    if (!paths.complete) {
        result.error = "Selected harmonization could not form continuous voice paths.";
        return result;
    }
    result.selectedVoicePaths = paths.paths;
    result.complete = true;
    return result;
}

} // namespace vll::harmony
