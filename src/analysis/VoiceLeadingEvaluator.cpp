#include "vll/analysis/VoiceLeadingEvaluator.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace vll::analysis {
namespace {

std::vector<VoiceTransition> orderedTransitions(const VoiceAssignment& assignment) {
    auto transitions = assignment.transitions;
    std::ranges::sort(transitions, {}, &VoiceTransition::voiceId);
    return transitions;
}

} // namespace

TransitionMetrics VoiceLeadingEvaluator::metrics(const VoiceAssignment& assignment) const {
    TransitionMetrics result;
    const auto transitions = orderedTransitions(assignment);
    for (const auto& transition : transitions) {
        const int movement = std::abs(transition.semitones);
        result.totalSemitoneDisplacement += movement;
        result.maximumVoiceMovement = std::max(result.maximumVoiceMovement, movement);
        if (movement == 0) ++result.stationaryVoices;
        else if (movement == 1) ++result.semitoneMoves;
        else if (movement == 2) ++result.wholeStepMoves;
        else ++result.leaps;
    }

    for (std::size_t lower = 0; lower < transitions.size(); ++lower) {
        for (std::size_t upper = lower + 1; upper < transitions.size(); ++upper) {
            if (transitions[lower].from < transitions[upper].from &&
                transitions[lower].to > transitions[upper].to) {
                result.hasCrossing = true;
            }
        }
    }
    for (std::size_t index = 1; index < transitions.size(); ++index) {
        const auto& lower = transitions[index - 1];
        const auto& upper = transitions[index];
        if (lower.to > upper.from || upper.to < lower.from) result.hasOverlap = true;
        result.maximumAdjacentSpacing = std::max(
            result.maximumAdjacentSpacing, std::abs(lower.to.distanceTo(upper.to)));
    }
    if (!transitions.empty()) {
        result.bassMovement = transitions.front().semitones;
        result.sopranoMovement = transitions.back().semitones;
    }
    return result;
}

AttemptResult VoiceLeadingEvaluator::evaluate(const VoiceAssignment& assignment,
                                              const ExerciseConstraint& constraints) const {
    AttemptResult result;
    const auto transitions = orderedTransitions(assignment);
    bool valid = transitions.size() == static_cast<std::size_t>(constraints.voiceCount);
    if (!valid) {
        result.observations.push_back({"voice_count_mismatch", "The assigned voice count does not match the exercise."});
    }

    for (const auto& transition : transitions) {
        if (std::abs(transition.semitones) > constraints.maximumLeap) {
            valid = false;
            std::ostringstream fact;
            fact << "Voice " << transition.voiceId << " moved "
                 << std::abs(transition.semitones) << " semitones; the limit is "
                 << constraints.maximumLeap << '.';
            result.observations.push_back({"maximum_leap_exceeded", fact.str()});
        }
    }
    if (!transitions.empty() && constraints.lockedBass &&
        transitions.front().to != *constraints.lockedBass) {
        valid = false;
        result.observations.push_back({"bass_lock_failed", "The destination bass does not match the locked bass."});
    }
    if (!transitions.empty() && constraints.lockedSoprano &&
        transitions.back().to != *constraints.lockedSoprano) {
        valid = false;
        result.observations.push_back({"soprano_lock_failed", "The destination soprano does not match the locked soprano."});
    }
    if (assignment.ambiguous) {
        result.observations.push_back({"ambiguous_assignment", "Two or more voice assignments have nearly equal cost."});
    }
    result.satisfiesConstraints = valid;
    return result;
}

} // namespace vll::analysis
