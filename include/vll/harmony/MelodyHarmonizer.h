#pragma once

#include "vll/core/Types.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace vll::harmony {

struct MelodyHarmonyEvent {
    Pitch melody;
    std::string chordSymbol;
    std::vector<int> chordPitchClasses;
};

struct HarmonizationRequest {
    std::vector<MelodyHarmonyEvent> events;
    int voiceCount{4};
    int minimumMidi{36};
    int maximumMidi{84};
    std::optional<Sonority> initialVoicing;
    std::size_t candidatesPerEvent{3};
};

struct HarmonizationCandidate {
    Sonority voicing;
    int totalDisplacement{0};
    int maximumMovement{0};
};

struct HarmonizedEvent {
    MelodyHarmonyEvent source;
    std::vector<HarmonizationCandidate> candidates;
    std::size_t selectedCandidate{0};
};

struct HarmonizationResult {
    bool complete{false};
    std::vector<HarmonizedEvent> events;
    std::vector<VoicePath> selectedVoicePaths;
    std::string error;
};

class MelodyHarmonizer final {
public:
    [[nodiscard]] HarmonizationResult harmonize(const HarmonizationRequest& request) const;
};

} // namespace vll::harmony
