#pragma once

#include "vll/core/Types.h"
#include <optional>
#include <string>
#include <vector>

namespace vll::visualization {

struct TimelineMarker {
    TimestampMicros timestamp{0};
    std::string label;
};

struct KeyboardKeyState {
    Pitch pitch{};
    VoiceId voiceId{0};
    bool isolated{false};
};

struct VoicePlaybackEvent {
    NoteEventType type{NoteEventType::NoteOn};
    VoiceId voiceId{0};
    Pitch pitch{};
    TimestampMicros timestamp{0};
};

struct Frame {
    std::vector<VoicePath> voicePaths;
    std::vector<Pitch> highlightedPitches;
    std::vector<KeyboardKeyState> keyboardKeys;
    std::vector<TimelineMarker> timeline;
    std::optional<VoiceId> isolatedVoice;
    TimestampMicros cursor{0};
    TimestampMicros visibleFrom{0};
    TimestampMicros visibleTo{0};
    double playbackRate{1.0};
    bool playing{false};
};

class IVisualizationModel {
public:
    virtual ~IVisualizationModel() = default;
    [[nodiscard]] virtual Frame frame() const = 0;
    virtual void isolateVoice(std::optional<VoiceId> voiceId) = 0;
    virtual void setCursor(TimestampMicros timestamp) = 0;
    virtual void setViewport(TimestampMicros from, TimestampMicros to) = 0;
    virtual void resetViewport() = 0;
    virtual void setPlaybackRate(double rate) = 0;
    virtual void setPlaying(bool playing) = 0;
    virtual void advancePlayback(TimestampMicros elapsedRealTime) = 0;
    [[nodiscard]] virtual std::vector<VoicePlaybackEvent> playbackEvents(
        TimestampMicros releaseTail = 500'000) const = 0;
};

} // namespace vll::visualization
