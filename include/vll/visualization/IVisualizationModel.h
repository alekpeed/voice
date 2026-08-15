#pragma once

#include "vll/core/Types.h"
#include <optional>
#include <vector>

namespace vll::visualization {

struct Frame {
    std::vector<VoicePath> voicePaths;
    std::vector<Pitch> highlightedPitches;
    std::optional<VoiceId> isolatedVoice;
    TimestampMicros cursor{0};
};

class IVisualizationModel {
public:
    virtual ~IVisualizationModel() = default;
    [[nodiscard]] virtual Frame frame() const = 0;
    virtual void isolateVoice(std::optional<VoiceId> voiceId) = 0;
    virtual void setCursor(TimestampMicros timestamp) = 0;
};

} // namespace vll::visualization
