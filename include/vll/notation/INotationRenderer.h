#pragma once

#include "vll/core/Types.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace vll::notation {

enum class AccidentalPreference { Sharps, Flats };

struct NotationNote {
    Pitch pitch{};
    VoiceId voiceId{0};
    std::optional<int> fingering;
    bool highlighted{false};
};

struct NotationEvent {
    TimestampMicros timestamp{0};
    std::vector<NotationNote> notes;
    std::string chordSymbol;
    double durationBeats{1.0};
};

struct NotationDocument {
    std::vector<NotationEvent> events;
    std::optional<TimestampMicros> playbackCursor;
};

struct EngravingOptions {
    float scale{1.0F};
    bool showChordSymbols{true};
    bool showAnalysisMarks{true};
    bool showFingering{true};
    bool showVoiceHighlights{true};
    std::optional<VoiceId> highlightedVoice;
    AccidentalPreference accidentalPreference{AccidentalPreference::Sharps};
    float maximumSystemWidth{960.0F};
};

struct RenderedNotation {
    std::string scalableVectorData;
    float width{0.0F};
    float height{0.0F};
    std::size_t systemCount{0};
    float contentWidth{0.0F};
};

class INotationRenderer {
public:
    virtual ~INotationRenderer() = default;
    [[nodiscard]] virtual RenderedNotation render(const NotationDocument& document,
                                                   const EngravingOptions& options) const = 0;
};

} // namespace vll::notation
