#include "vll/notation/NotationLayout.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace vll::notation {
namespace {

struct SpelledPitch {
    int letter{0};
    int octave{4};
    int accidental{0};
    std::string name;
};

SpelledPitch spell(const Pitch pitch, const AccidentalPreference preference) {
    static constexpr std::array<int, 12> sharpLetters{0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};
    static constexpr std::array<int, 12> sharpAccidentals{0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    static constexpr std::array<int, 12> flatLetters{0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6};
    static constexpr std::array<int, 12> flatAccidentals{0, -1, 0, -1, 0, 0, -1, 0, -1, 0, -1, 0};
    static constexpr std::array<char, 7> letterNames{'C', 'D', 'E', 'F', 'G', 'A', 'B'};

    const int pitchClass = pitch.pitchClass();
    const bool flats = preference == AccidentalPreference::Flats;
    const int letter = flats ? flatLetters[static_cast<std::size_t>(pitchClass)]
                             : sharpLetters[static_cast<std::size_t>(pitchClass)];
    const int accidental = flats ? flatAccidentals[static_cast<std::size_t>(pitchClass)]
                                 : sharpAccidentals[static_cast<std::size_t>(pitchClass)];
    const int octave = pitch.midiNote / 12 - 1;
    std::string name(1, letterNames[static_cast<std::size_t>(letter)]);
    if (accidental > 0) name += '#';
    else if (accidental < 0) name += 'b';
    name += std::to_string(octave);
    return {letter, octave, accidental, std::move(name)};
}

Staff chooseStaff(const NotationNote& note, const VoiceId maximumVoiceId) {
    if (note.voiceId == 0) return note.pitch.midiNote >= 60 ? Staff::Treble : Staff::Bass;
    if (maximumVoiceId >= 4) return note.voiceId <= 2 ? Staff::Bass : Staff::Treble;
    if (maximumVoiceId == 3) {
        if (note.voiceId == 1) return Staff::Bass;
        if (note.voiceId == 3) return Staff::Treble;
        return note.pitch.midiNote >= 60 ? Staff::Treble : Staff::Bass;
    }
    return note.voiceId == 1 ? Staff::Bass : Staff::Treble;
}

float noteY(const SpelledPitch& pitch, const Staff staff,
            const float trebleBottom, const float bassBottom, const float scale) {
    const int diatonic = pitch.octave * 7 + pitch.letter;
    const int reference = staff == Staff::Treble ? 4 * 7 + 2 : 2 * 7 + 4;
    const float bottom = staff == Staff::Treble ? trebleBottom : bassBottom;
    return bottom - static_cast<float>(diatonic - reference) * 5.0F * scale;
}

std::vector<float> ledgerLines(const float y, const float staffTop,
                               const float staffBottom, const float scale) {
    std::vector<float> lines;
    const float lineStep = 10.0F * scale;
    for (float ledger = staffTop - lineStep; ledger >= y - 0.1F; ledger -= lineStep) {
        lines.push_back(ledger);
    }
    for (float ledger = staffBottom + lineStep; ledger <= y + 0.1F; ledger += lineStep) {
        lines.push_back(ledger);
    }
    return lines;
}

void resolveNoteheadCollisions(std::vector<NoteGlyph>& notes, const float scale) {
    std::vector<std::size_t> order(notes.size());
    for (std::size_t index = 0; index < order.size(); ++index) order[index] = index;
    std::ranges::sort(order, [&](const std::size_t left, const std::size_t right) {
        return notes[left].y < notes[right].y;
    });
    for (std::size_t position = 1; position < order.size(); ++position) {
        auto& current = notes[order[position]];
        const auto& previous = notes[order[position - 1]];
        if (current.eventIndex == previous.eventIndex && current.staff == previous.staff &&
            std::abs(current.y - previous.y) <= 5.1F * scale) {
            current.noteheadOffset = previous.noteheadOffset == 0.0F ? 8.0F * scale : 0.0F;
        }
    }
}

void resolveAccidentalColumns(std::vector<NoteGlyph>& notes, const float scale) {
    std::map<std::pair<std::size_t, Staff>, std::vector<float>> columnsByChord;
    std::vector<std::size_t> order(notes.size());
    for (std::size_t index = 0; index < order.size(); ++index) order[index] = index;
    std::ranges::sort(order, [&](const std::size_t left, const std::size_t right) {
        return notes[left].y < notes[right].y;
    });
    for (const auto index : order) {
        auto& note = notes[index];
        if (note.accidental.empty()) continue;
        auto& columns = columnsByChord[{note.eventIndex, note.staff}];
        int column = 0;
        while (column < static_cast<int>(columns.size()) &&
               std::abs(note.y - columns[static_cast<std::size_t>(column)]) < 12.0F * scale) {
            ++column;
        }
        if (column == static_cast<int>(columns.size())) columns.push_back(note.y);
        else columns[static_cast<std::size_t>(column)] = note.y;
        note.accidentalColumn = column;
    }
}

} // namespace

NotationLayout NotationLayouter::layout(const NotationDocument& document,
                                        const EngravingOptions& incomingOptions) const {
    NotationLayout result;
    if (document.events.empty()) return result;

    const float scale = std::clamp(incomingOptions.scale, 0.5F, 2.0F);
    const float maximumWidth = std::clamp(incomingOptions.maximumSystemWidth, 320.0F, 1600.0F);
    const float leftMargin = 140.0F * scale;
    const float rightMargin = 24.0F * scale;
    const float eventSpacing = 86.0F * scale;
    const float systemHeight = 250.0F * scale;
    const float available = std::max(eventSpacing, maximumWidth - leftMargin - rightMargin);
    const std::size_t eventsPerSystem = std::max<std::size_t>(
        1, static_cast<std::size_t>(std::floor(available / eventSpacing)));
    const std::size_t systemCount = (document.events.size() + eventsPerSystem - 1) / eventsPerSystem;

    VoiceId maximumVoiceId = 0;
    for (const auto& event : document.events) {
        for (const auto& note : event.notes) maximumVoiceId = std::max(maximumVoiceId, note.voiceId);
    }

    result.scale = scale;
    result.height = static_cast<float>(systemCount) * systemHeight;
    result.systems.reserve(systemCount);
    for (std::size_t systemIndex = 0; systemIndex < systemCount; ++systemIndex) {
        SystemLayout system;
        system.firstEvent = systemIndex * eventsPerSystem;
        system.eventCount = std::min(eventsPerSystem, document.events.size() - system.firstEvent);
        system.top = static_cast<float>(systemIndex) * systemHeight;
        system.width = leftMargin + static_cast<float>(system.eventCount) * eventSpacing + rightMargin;
        result.width = std::max(result.width, system.width);
        result.contentWidth = std::max(result.contentWidth, system.width);

        const float trebleTop = system.top + 60.0F * scale;
        const float trebleBottom = system.top + 100.0F * scale;
        const float bassTop = system.top + 150.0F * scale;
        const float bassBottom = system.top + 190.0F * scale;
        std::map<std::pair<int, int>, int> accidentalState;

        for (std::size_t localIndex = 0; localIndex < system.eventCount; ++localIndex) {
            const std::size_t eventIndex = system.firstEvent + localIndex;
            const auto& event = document.events[eventIndex];
            const float x = leftMargin + (static_cast<float>(localIndex) + 0.5F) * eventSpacing;
            system.events.push_back({eventIndex, x, event.timestamp,
                                     incomingOptions.showChordSymbols ? event.chordSymbol : std::string{}});
            for (const auto& note : event.notes) {
                const auto spelled = spell(note.pitch, incomingOptions.accidentalPreference);
                const Staff staff = chooseStaff(note, maximumVoiceId);
                const float y = noteY(spelled, staff, trebleBottom, bassBottom, scale);
                const auto stateKey = std::pair{spelled.letter, spelled.octave};
                const int currentAccidental = accidentalState.contains(stateKey)
                                                  ? accidentalState[stateKey]
                                                  : 0;
                std::string accidental;
                if (spelled.accidental != currentAccidental) {
                    accidental = spelled.accidental > 0 ? "sharp"
                                 : spelled.accidental < 0 ? "flat" : "natural";
                    accidentalState[stateKey] = spelled.accidental;
                }
                const float staffTop = staff == Staff::Treble ? trebleTop : bassTop;
                const float staffBottom = staff == Staff::Treble ? trebleBottom : bassBottom;
                const bool highlighted = incomingOptions.showVoiceHighlights &&
                    (note.highlighted || (incomingOptions.highlightedVoice &&
                                          note.voiceId == *incomingOptions.highlightedVoice));
                system.notes.push_back({
                    eventIndex, staff, note.pitch, note.voiceId, x, y, 0.0F,
                    spelled.name, std::move(accidental), 0,
                    ledgerLines(y, staffTop, staffBottom, scale),
                    incomingOptions.showFingering ? note.fingering : std::nullopt,
                    highlighted, std::max(0.25, event.durationBeats)});
            }
        }
        resolveNoteheadCollisions(system.notes, scale);
        resolveAccidentalColumns(system.notes, scale);
        result.systems.push_back(std::move(system));
    }

    if (document.playbackCursor) {
        const TimestampMicros cursor = std::clamp(*document.playbackCursor,
                                                  document.events.front().timestamp,
                                                  document.events.back().timestamp);
        auto next = std::ranges::upper_bound(document.events, cursor, {}, &NotationEvent::timestamp);
        std::size_t leftIndex = next == document.events.begin()
                                    ? 0
                                    : static_cast<std::size_t>(std::distance(document.events.begin(), next) - 1);
        std::size_t rightIndex = std::min(leftIndex + 1, document.events.size() - 1);
        const std::size_t systemIndex = leftIndex / eventsPerSystem;
        auto& system = result.systems[systemIndex];
        const auto& leftEvent = system.events[leftIndex - system.firstEvent];
        float cursorX = leftEvent.x;
        if (rightIndex != leftIndex && rightIndex / eventsPerSystem == systemIndex) {
            const auto& rightEvent = system.events[rightIndex - system.firstEvent];
            const auto duration = document.events[rightIndex].timestamp - document.events[leftIndex].timestamp;
            const double ratio = duration > 0
                                     ? static_cast<double>(cursor - document.events[leftIndex].timestamp) /
                                           static_cast<double>(duration)
                                     : 0.0;
            cursorX = leftEvent.x + static_cast<float>(ratio) * (rightEvent.x - leftEvent.x);
        }
        system.playbackCursorX = cursorX;
    }
    return result;
}

} // namespace vll::notation
