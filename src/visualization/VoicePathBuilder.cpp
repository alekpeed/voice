#include "vll/visualization/VoicePathBuilder.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

namespace vll::visualization {
namespace {

struct ActiveVoice {
    Pitch pitch{};
    VoiceId id{0};
};

std::vector<Pitch> normalized(const Sonority& sonority) {
    auto pitches = sonority.pitches;
    std::ranges::sort(pitches);
    const auto duplicate = std::ranges::unique(pitches);
    pitches.erase(duplicate.begin(), duplicate.end());
    return pitches;
}

std::string voiceLabel(const std::size_t voiceCount, const std::size_t index) {
    if (voiceCount == 2) return index == 0 ? "Lower" : "Upper";
    if (voiceCount == 3) {
        if (index == 0) return "Bass";
        if (index == 1) return "Inner";
        return "Soprano";
    }
    static constexpr const char* labels[]{"Bass", "Tenor", "Alto", "Soprano"};
    return labels[index];
}

} // namespace

VoicePathBuilder::VoicePathBuilder(const analysis::IVoiceAssigner& assigner) : assigner_(assigner) {}

VoicePathBuildResult VoicePathBuilder::build(const std::vector<Sonority>& sonorities,
                                             const ExerciseConstraint& constraints) const {
    VoicePathBuildResult result;
    if (sonorities.empty()) return result;
    for (std::size_t index = 1; index < sonorities.size(); ++index) {
        if (sonorities[index].startedAt <= sonorities[index - 1].startedAt) {
            result.failedTransition = index - 1;
            return result;
        }
    }

    const auto firstPitches = normalized(sonorities.front());
    if (firstPitches.size() != static_cast<std::size_t>(constraints.voiceCount) ||
        firstPitches.size() < 2 || firstPitches.size() > 4) {
        return result;
    }

    std::vector<ActiveVoice> active;
    active.reserve(firstPitches.size());
    result.paths.reserve(firstPitches.size());
    for (std::size_t index = 0; index < firstPitches.size(); ++index) {
        const VoiceId id = static_cast<VoiceId>(index + 1);
        active.push_back({firstPitches[index], id});
        result.paths.push_back({Voice{id, voiceLabel(firstPitches.size(), index)},
                                {{id, firstPitches[index], sonorities.front().startedAt}}});
    }

    for (std::size_t transitionIndex = 1; transitionIndex < sonorities.size(); ++transitionIndex) {
        Sonority source;
        source.startedAt = sonorities[transitionIndex - 1].startedAt;
        source.endedAt = sonorities[transitionIndex - 1].endedAt;
        for (const auto& voice : active) source.pitches.push_back(voice.pitch);

        auto assignment = assigner_.assign(source, sonorities[transitionIndex], constraints);
        if (assignment.transitions.size() != active.size()) {
            result.failedTransition = transitionIndex - 1;
            return result;
        }

        std::vector<ActiveVoice> destination;
        destination.reserve(active.size());
        for (auto& transition : assignment.transitions) {
            const std::size_t sourceRank = static_cast<std::size_t>(transition.voiceId - 1);
            if (sourceRank >= active.size()) {
                result.failedTransition = transitionIndex - 1;
                return result;
            }
            const VoiceId persistentId = active[sourceRank].id;
            transition.voiceId = persistentId;
            result.paths[static_cast<std::size_t>(persistentId - 1)].points.push_back(
                {persistentId, transition.to, sonorities[transitionIndex].startedAt});
            destination.push_back({transition.to, persistentId});
        }
        std::ranges::sort(destination, [](const ActiveVoice& left, const ActiveVoice& right) {
            if (left.pitch != right.pitch) return left.pitch < right.pitch;
            return left.id < right.id;
        });
        active = std::move(destination);
        result.assignments.push_back(std::move(assignment));
    }

    result.complete = true;
    result.failedTransition = sonorities.size() - 1;
    return result;
}

} // namespace vll::visualization
