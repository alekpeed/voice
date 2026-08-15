#include "vll/lab/ReharmonizationLab.h"

#include "vll/exercise/NearestVoicingSolver.h"
#include "vll/analysis/VoiceAssigner.h"
#include "vll/visualization/VoicePathBuilder.h"

#include <algorithm>

namespace vll::lab {

LabRealization ReharmonizationLab::realize(const LabRequest& request) const {
    LabRealization result;
    const auto eventCount = request.harmony.size();
    if (request.id.empty() || eventCount == 0 || request.voiceCount < 2 ||
        request.voiceCount > 4 || static_cast<int>(request.initialVoicing.pitches.size()) != request.voiceCount ||
        (!request.sopranoLine.empty() && request.sopranoLine.size() != eventCount) ||
        (!request.bassLine.empty() && request.bassLine.size() != eventCount)) {
        result.error = "Lab request dimensions are invalid.";
        return result;
    }
    for (const auto& line : request.requiredInnerLines) {
        if (line.size() != eventCount) {
            result.error = "Each required inner line must span every harmony event.";
            return result;
        }
    }
    result.voicings.push_back(request.initialVoicing);
    result.voicings.front().startedAt = 0;
    result.voicings.front().endedAt = 800'000;
    const exercise::NearestVoicingSolver solver;
    for (std::size_t index = 1; index < eventCount; ++index) {
        std::vector<int> required;
        for (const auto& line : request.requiredInnerLines) required.push_back(line[index].pitchClass());
        const auto bass = request.bassLine.empty() ? std::optional<Pitch>{} : request.bassLine[index];
        const auto soprano = request.sopranoLine.empty() ? std::optional<Pitch>{} : request.sopranoLine[index];
        const auto solution = solver.solve({
            result.voicings.back(), request.harmony[index].pitchClasses, required,
            request.voiceCount, 24, 96, 12, 19, bass, soprano});
        if (!solution.found) {
            result.error = "No realization satisfies the fixed lines at event " +
                           std::to_string(index + 1) + ".";
            return result;
        }
        auto destination = solution.destination;
        destination.startedAt = static_cast<TimestampMicros>(index) * 1'000'000;
        destination.endedAt = destination.startedAt + 800'000;
        result.totalDisplacement += solution.metrics.totalSemitoneDisplacement;
        result.voicings.push_back(std::move(destination));
    }
    ExerciseConstraint constraints;
    constraints.voiceCount = request.voiceCount;
    constraints.maximumLeap = 12;
    analysis::VoiceAssigner assigner;
    const auto paths = visualization::VoicePathBuilder(assigner).build(result.voicings, constraints);
    if (!paths.complete) {
        result.error = "The realization does not form continuous voice paths.";
        return result;
    }
    result.voicePaths = paths.paths;
    result.complete = true;
    return result;
}

bool ReharmonizationLab::save(const std::string& name, LabRequest request) {
    if (name.empty() || request.id.empty()) return false;
    saved_[name] = std::move(request);
    return true;
}

std::optional<LabRequest> ReharmonizationLab::load(const std::string& name) const {
    const auto found = saved_.find(name);
    return found == saved_.end() ? std::nullopt : std::optional<LabRequest>{found->second};
}

std::vector<std::string> ReharmonizationLab::savedNames() const {
    std::vector<std::string> names;
    names.reserve(saved_.size());
    for (const auto& [name, ignored] : saved_) {
        static_cast<void>(ignored);
        names.push_back(name);
    }
    std::ranges::sort(names);
    return names;
}

} // namespace vll::lab
