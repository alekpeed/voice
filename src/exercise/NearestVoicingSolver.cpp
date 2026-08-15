#include "vll/exercise/NearestVoicingSolver.h"

#include "vll/analysis/VoiceAssigner.h"
#include "vll/analysis/VoiceLeadingEvaluator.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

namespace vll::exercise {
namespace {

int normalizedPitchClass(const int pitchClass) {
    const int value = pitchClass % 12;
    return value < 0 ? value + 12 : value;
}

bool containsPitchClass(const std::vector<Pitch>& pitches, const int pitchClass) {
    return std::ranges::any_of(pitches, [&](const Pitch pitch) {
        return pitch.pitchClass() == normalizedPitchClass(pitchClass);
    });
}

bool isBetter(const analysis::TransitionMetrics& candidateMetrics,
              const std::vector<Pitch>& candidatePitches,
              const analysis::TransitionMetrics& bestMetrics,
              const std::vector<Pitch>& bestPitches) {
    if (candidateMetrics.totalSemitoneDisplacement != bestMetrics.totalSemitoneDisplacement) {
        return candidateMetrics.totalSemitoneDisplacement < bestMetrics.totalSemitoneDisplacement;
    }
    if (candidateMetrics.maximumVoiceMovement != bestMetrics.maximumVoiceMovement) {
        return candidateMetrics.maximumVoiceMovement < bestMetrics.maximumVoiceMovement;
    }
    const int candidateSpan = candidatePitches.front().distanceTo(candidatePitches.back());
    const int bestSpan = bestPitches.front().distanceTo(bestPitches.back());
    if (candidateSpan != bestSpan) return candidateSpan < bestSpan;
    return candidatePitches < bestPitches;
}

} // namespace

VoicingSolution NearestVoicingSolver::solve(const VoicingSearchRequest& request) const {
    VoicingSolution best;
    if (request.voiceCount < 2 || request.voiceCount > 4 ||
        request.source.pitches.size() != static_cast<std::size_t>(request.voiceCount) ||
        request.allowedPitchClasses.empty() || request.minimumMidi > request.maximumMidi) {
        return best;
    }

    std::vector<Pitch> pool;
    for (int midiNote = request.minimumMidi; midiNote <= request.maximumMidi; ++midiNote) {
        if (std::ranges::any_of(request.allowedPitchClasses, [&](const int pitchClass) {
                return normalizedPitchClass(pitchClass) == Pitch{midiNote}.pitchClass();
            })) {
            pool.push_back(Pitch{midiNote});
        }
    }
    if (pool.size() < static_cast<std::size_t>(request.voiceCount)) return best;

    const analysis::VoiceAssigner assigner;
    const analysis::VoiceLeadingEvaluator evaluator;
    ExerciseConstraint constraints;
    constraints.voiceCount = request.voiceCount;
    constraints.maximumLeap = request.maximumLeap;
    constraints.lockedBass = request.lockedBass;
    constraints.lockedSoprano = request.lockedSoprano;
    std::vector<Pitch> candidate;
    candidate.reserve(static_cast<std::size_t>(request.voiceCount));

    const auto search = [&](auto&& self, const std::size_t poolIndex) -> void {
        if (candidate.size() == static_cast<std::size_t>(request.voiceCount)) {
            if (request.lockedBass && candidate.front() != *request.lockedBass) return;
            if (request.lockedSoprano && candidate.back() != *request.lockedSoprano) return;
            for (const int pitchClass : request.requiredPitchClasses) {
                if (!containsPitchClass(candidate, pitchClass)) return;
            }
            Sonority destination{candidate, request.source.startedAt + 1'000'000,
                                 request.source.startedAt + 1'080'000};
            const auto assignment = assigner.assign(request.source, destination, constraints);
            if (assignment.transitions.size() != candidate.size()) return;
            const auto metrics = evaluator.metrics(assignment);
            if (metrics.hasCrossing || metrics.maximumVoiceMovement > request.maximumLeap ||
                metrics.maximumAdjacentSpacing > request.maximumAdjacentSpacing) {
                return;
            }
            if (!best.found || isBetter(metrics, candidate, best.metrics,
                                        best.destination.pitches)) {
                best = {true, std::move(destination), assignment, metrics};
            }
            return;
        }
        const std::size_t remaining = static_cast<std::size_t>(request.voiceCount) - candidate.size();
        if (pool.size() - poolIndex < remaining) return;
        for (std::size_t index = poolIndex; index < pool.size(); ++index) {
            candidate.push_back(pool[index]);
            self(self, index + 1);
            candidate.pop_back();
        }
    };
    search(search, 0);
    return best;
}

} // namespace vll::exercise
