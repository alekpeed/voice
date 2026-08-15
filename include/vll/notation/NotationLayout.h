#pragma once

#include "vll/notation/INotationRenderer.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace vll::notation {

enum class Staff { Treble, Bass };

struct NoteGlyph {
    std::size_t eventIndex{0};
    Staff staff{Staff::Treble};
    Pitch pitch{};
    VoiceId voiceId{0};
    float x{0.0F};
    float y{0.0F};
    float noteheadOffset{0.0F};
    std::string pitchName;
    std::string accidental;
    int accidentalColumn{0};
    std::vector<float> ledgerLines;
    std::optional<int> fingering;
    bool highlighted{false};
    double durationBeats{1.0};
};

struct EventGlyph {
    std::size_t eventIndex{0};
    float x{0.0F};
    TimestampMicros timestamp{0};
    std::string chordSymbol;
};

struct SystemLayout {
    std::size_t firstEvent{0};
    std::size_t eventCount{0};
    float top{0.0F};
    float width{0.0F};
    std::vector<NoteGlyph> notes;
    std::vector<EventGlyph> events;
    std::optional<float> playbackCursorX;
};

struct NotationLayout {
    float width{0.0F};
    float height{0.0F};
    float contentWidth{0.0F};
    float scale{1.0F};
    std::vector<SystemLayout> systems;
};

class NotationLayouter final {
public:
    [[nodiscard]] NotationLayout layout(
        const NotationDocument& document,
        const EngravingOptions& options) const;
};

} // namespace vll::notation
