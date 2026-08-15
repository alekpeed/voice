#include "vll/analysis/DeterministicFeedback.h"

#include "vll/analysis/VoiceLeadingEvaluator.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace vll::analysis {
namespace {

std::vector<VoiceTransition> orderedTransitions(const VoiceAssignment& assignment) {
    auto transitions = assignment.transitions;
    std::ranges::sort(transitions, {}, &VoiceTransition::voiceId);
    return transitions;
}

int normalizedPitchClass(const int pitchClass) {
    const int value = pitchClass % 12;
    return value < 0 ? value + 12 : value;
}

bool containsPitchClass(const std::optional<ChordFeedbackContext>& chord, const Pitch pitch) {
    if (!chord) return false;
    return std::ranges::any_of(chord->guideTonePitchClasses, [&](const int pitchClass) {
        return normalizedPitchClass(pitchClass) == pitch.pitchClass();
    });
}

std::string chordName(const std::optional<ChordFeedbackContext>& chord,
                      const std::string_view fallback) {
    return chord && !chord->label.empty() ? chord->label : std::string{fallback};
}

std::string pitchName(const Pitch pitch, const PitchNamePreference preference) {
    static constexpr std::array<std::string_view, 12> sharps{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    static constexpr std::array<std::string_view, 12> flats{
        "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
    const auto& names = preference == PitchNamePreference::Flats ? flats : sharps;
    return std::string{names[static_cast<std::size_t>(pitch.pitchClass())]} +
           std::to_string(pitch.midiNote / 12 - 1);
}

std::string countPhrase(const int count, const std::string_view singular,
                        const std::string_view plural) {
    return std::to_string(count) + ' ' + std::string{count == 1 ? singular : plural};
}

Observation movementFact(const VoiceTransition& transition,
                         const PitchNamePreference preference) {
    const int distance = std::abs(transition.semitones);
    const std::string from = pitchName(transition.from, preference);
    const std::string to = pitchName(transition.to, preference);
    std::ostringstream fact;
    if (distance == 0) {
        fact << "Voice " << transition.voiceId << " retained " << from
             << " as a common tone.";
        return {"common_tone", fact.str()};
    }

    fact << "Voice " << transition.voiceId << " moved from " << from << " to " << to
         << ": " << (transition.semitones > 0 ? "ascending " : "descending ")
         << distance << (distance == 1 ? " semitone" : " semitones");
    if (distance == 2) fact << " (a whole step)";
    else if (distance > 2) fact << " (a leap)";
    fact << '.';
    const std::string code = distance == 1 ? "semitone_move"
                             : distance == 2 ? "whole_step_move" : "leap";
    return {code, fact.str()};
}

std::optional<Observation> guideToneFact(const VoiceTransition& transition,
                                         const FeedbackContext& context) {
    const bool sourceGuideTone = containsPitchClass(context.sourceChord, transition.from);
    const bool destinationGuideTone = containsPitchClass(context.destinationChord, transition.to);
    if (!sourceGuideTone && !destinationGuideTone) return std::nullopt;

    const std::string sourceName = chordName(context.sourceChord, "source chord");
    const std::string destinationName = chordName(context.destinationChord, "destination chord");
    const std::string from = pitchName(transition.from, context.pitchNames);
    const std::string to = pitchName(transition.to, context.pitchNames);
    std::ostringstream fact;
    if (sourceGuideTone && destinationGuideTone) {
        if (transition.semitones == 0) {
            fact << "Voice " << transition.voiceId << " retained " << from
                 << " as a guide tone in both " << sourceName << " and "
                 << destinationName << '.';
        } else {
            fact << "Voice " << transition.voiceId << " moved from " << sourceName
                 << " guide tone " << from << " to " << destinationName
                 << " guide tone " << to << '.';
        }
        return Observation{"guide_tone_connection", fact.str()};
    }
    if (destinationGuideTone) {
        fact << "Voice " << transition.voiceId << " arrived on " << destinationName
             << " guide tone " << to << " from " << from << '.';
        return Observation{"guide_tone_arrival", fact.str()};
    }
    fact << "Voice " << transition.voiceId << " departed " << sourceName
         << " guide tone " << from << " and ended on " << to << '.';
    return Observation{"guide_tone_departure", fact.str()};
}

Observation summaryFact(const TransitionMetrics& metrics, const FeedbackContext& context) {
    const std::string source = chordName(context.sourceChord, "source sonority");
    const std::string destination = chordName(context.destinationChord, "destination sonority");
    const int stepwise = metrics.semitoneMoves + metrics.wholeStepMoves;
    std::ostringstream fact;
    fact << "From " << source << " to " << destination << ": "
         << countPhrase(metrics.stationaryVoices, "common tone", "common tones") << ", "
         << countPhrase(stepwise, "stepwise move", "stepwise moves") << ", "
         << countPhrase(metrics.leaps, "leap", "leaps") << "; total displacement "
         << countPhrase(metrics.totalSemitoneDisplacement, "semitone", "semitones") << '.';
    return {"transition_summary", fact.str()};
}

std::vector<Observation> crossingFacts(const std::vector<VoiceTransition>& transitions,
                                       const PitchNamePreference preference) {
    std::vector<Observation> observations;
    for (std::size_t lower = 0; lower < transitions.size(); ++lower) {
        for (std::size_t upper = lower + 1; upper < transitions.size(); ++upper) {
            const auto& first = transitions[lower];
            const auto& second = transitions[upper];
            if (!((first.from < second.from && first.to > second.to) ||
                  (first.from > second.from && first.to < second.to))) {
                continue;
            }
            const auto& endingAbove = first.to > second.to ? first : second;
            const auto& endingBelow = first.to > second.to ? second : first;
            std::ostringstream fact;
            fact << "Voices " << first.voiceId << " and " << second.voiceId
                 << " crossed: voice " << endingAbove.voiceId << " ended on "
                 << pitchName(endingAbove.to, preference) << " above voice "
                 << endingBelow.voiceId << " on " << pitchName(endingBelow.to, preference) << '.';
            observations.push_back({"voice_crossing", fact.str()});
        }
    }
    return observations;
}

} // namespace

FeedbackReport DeterministicFeedbackGenerator::generate(
    const VoiceAssignment& assignment, const FeedbackContext& context) const {
    FeedbackReport report;
    report.metrics = VoiceLeadingEvaluator{}.metrics(assignment);
    const auto transitions = orderedTransitions(assignment);
    if (transitions.empty()) {
        report.observations.push_back(
            {"no_voice_transitions", "No assigned voice transitions were available for feedback."});
        return report;
    }

    report.observations.push_back(summaryFact(report.metrics, context));
    for (const auto& transition : transitions) {
        report.observations.push_back(movementFact(transition, context.pitchNames));
        if (const auto guideTone = guideToneFact(transition, context)) {
            report.observations.push_back(*guideTone);
        }
    }
    auto crossings = crossingFacts(transitions, context.pitchNames);
    report.observations.insert(report.observations.end(), crossings.begin(), crossings.end());
    if (assignment.ambiguous) {
        report.observations.push_back(
            {"ambiguous_assignment", "Two or more voice assignments have nearly equal cost."});
    }
    return report;
}

} // namespace vll::analysis
