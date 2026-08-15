#include "vll/analysis/VoiceAssigner.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

namespace vll::analysis {
namespace {

struct CandidateAssignment {
    double cost{std::numeric_limits<double>::infinity()};
    std::array<std::size_t, 4> destinationIndexes{};
};

std::vector<Pitch> normalizedPitches(const Sonority& sonority) {
    auto pitches = sonority.pitches;
    std::ranges::sort(pitches);
    const auto duplicate = std::ranges::unique(pitches);
    pitches.erase(duplicate.begin(), duplicate.end());
    return pitches;
}

bool constraintsCanMatch(const std::vector<Pitch>& destination,
                         const ExerciseConstraint& constraints) {
    if (destination.size() != static_cast<std::size_t>(constraints.voiceCount)) return false;
    if (constraints.lockedBass && destination.front() != *constraints.lockedBass) return false;
    if (constraints.lockedSoprano && destination.back() != *constraints.lockedSoprano) return false;
    return true;
}

double assignmentCost(const std::vector<Pitch>& source,
                      const std::vector<Pitch>& destination,
                      const std::array<std::size_t, 4>& permutation,
                      const ExerciseConstraint& constraints,
                      const VoiceAssignmentWeights& weights) {
    double cost = 0.0;
    for (std::size_t sourceIndex = 0; sourceIndex < source.size(); ++sourceIndex) {
        const int movement = std::abs(source[sourceIndex].distanceTo(destination[permutation[sourceIndex]]));
        cost += static_cast<double>(movement) * weights.pitchDistance;
        if (movement == 0) cost -= weights.commonToneBonus;
        if (movement > constraints.maximumLeap) {
            cost += static_cast<double>(movement - constraints.maximumLeap) * weights.leapPenalty;
        }
    }

    for (std::size_t lower = 0; lower < source.size(); ++lower) {
        for (std::size_t upper = lower + 1; upper < source.size(); ++upper) {
            if (destination[permutation[lower]] > destination[permutation[upper]]) {
                cost += weights.crossingPenalty;
            }
        }
    }
    return cost;
}

} // namespace

VoiceAssigner::VoiceAssigner(VoiceAssignmentWeights weights) : weights_(weights) {
    weights_.pitchDistance = std::max(0.0, weights_.pitchDistance);
    weights_.commonToneBonus = std::max(0.0, weights_.commonToneBonus);
    weights_.crossingPenalty = std::max(0.0, weights_.crossingPenalty);
    weights_.leapPenalty = std::max(0.0, weights_.leapPenalty);
    weights_.ambiguityTolerance = std::max(0.0, weights_.ambiguityTolerance);
}

VoiceAssignment VoiceAssigner::assign(const Sonority& sourceSonority,
                                      const Sonority& destinationSonority,
                                      const ExerciseConstraint& constraints) const {
    const auto source = normalizedPitches(sourceSonority);
    const auto destination = normalizedPitches(destinationSonority);
    if (source.size() != destination.size() || source.size() < 2 || source.size() > 4 ||
        constraints.voiceCount < 2 || constraints.voiceCount > 4 ||
        !constraintsCanMatch(destination, constraints)) {
        return VoiceAssignment{{}, 0.0, true};
    }

    std::array<std::size_t, 4> permutation{0, 1, 2, 3};
    std::vector<CandidateAssignment> candidates;
    do {
        if (constraints.lockedBass && destination[permutation[0]] != *constraints.lockedBass) continue;
        if (constraints.lockedSoprano &&
            destination[permutation[source.size() - 1]] != *constraints.lockedSoprano) continue;
        candidates.push_back({assignmentCost(source, destination, permutation, constraints, weights_), permutation});
    } while (std::next_permutation(permutation.begin(),
                                   std::next(permutation.begin(), static_cast<std::ptrdiff_t>(source.size()))));

    if (candidates.empty()) return VoiceAssignment{{}, 0.0, true};
    std::ranges::sort(candidates, [](const CandidateAssignment& left, const CandidateAssignment& right) {
        if (left.cost != right.cost) return left.cost < right.cost;
        return left.destinationIndexes < right.destinationIndexes;
    });

    const auto& best = candidates.front();
    const double secondCost = candidates.size() > 1 ? candidates[1].cost
                                                     : std::numeric_limits<double>::infinity();
    const double gap = secondCost - best.cost;
    const bool ambiguous = std::isfinite(secondCost) && gap <= weights_.ambiguityTolerance;
    const double normalizedCost = std::max(0.0, best.cost) /
                                  (12.0 * static_cast<double>(source.size()));
    const double quality = 1.0 / (1.0 + normalizedCost);
    const double separation = std::isfinite(secondCost)
                                  ? std::clamp(gap / (std::abs(secondCost) + 1.0), 0.0, 1.0)
                                  : 1.0;
    double confidence = std::clamp(0.65 * quality + 0.35 * separation, 0.0, 1.0);
    if (ambiguous) confidence *= 0.5;

    std::vector<VoiceTransition> transitions;
    transitions.reserve(source.size());
    for (std::size_t index = 0; index < source.size(); ++index) {
        const Pitch from = source[index];
        const Pitch to = destination[best.destinationIndexes[index]];
        const int semitones = from.distanceTo(to);
        transitions.push_back(VoiceTransition{static_cast<VoiceId>(index + 1), from, to,
                                              semitones, classifyMovement(semitones)});
    }
    return VoiceAssignment{std::move(transitions), confidence, ambiguous};
}

VoiceAssignmentWeights VoiceAssigner::weights() const noexcept { return weights_; }

} // namespace vll::analysis
