#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace vll {

using TimestampMicros = std::int64_t;
using VoiceId = std::uint32_t;

struct Pitch {
    int midiNote{60};

    [[nodiscard]] int pitchClass() const noexcept {
        const int value = midiNote % 12;
        return value < 0 ? value + 12 : value;
    }

    [[nodiscard]] int distanceTo(const Pitch other) const noexcept {
        return other.midiNote - midiNote;
    }

    auto operator<=>(const Pitch&) const = default;
};

enum class NoteEventType { NoteOn, NoteOff, SustainOn, SustainOff };

struct NoteEvent {
    NoteEventType type{NoteEventType::NoteOn};
    Pitch pitch{};
    float velocity{0.0F};
    TimestampMicros timestamp{0};
    int channel{1};
};

struct Sonority {
    std::vector<Pitch> pitches;
    TimestampMicros startedAt{0};
    TimestampMicros endedAt{0};
};

struct Voice {
    VoiceId id{0};
    std::string label;
};

struct VoicePoint {
    VoiceId voiceId{0};
    Pitch pitch{};
    TimestampMicros timestamp{0};
};

struct VoicePath {
    Voice voice;
    std::vector<VoicePoint> points;
};

enum class MotionType { Stationary, Oblique, Similar, Parallel, Contrary };
enum class MovementSize { Stationary, Semitone, WholeStep, Leap };

struct VoiceTransition {
    VoiceId voiceId{0};
    Pitch from{};
    Pitch to{};
    int semitones{0};
    MovementSize size{MovementSize::Stationary};
};

struct ChordCandidate {
    std::string symbol;
    std::optional<Pitch> root;
    double confidence{0.0};
};

struct ExerciseConstraint {
    std::optional<Pitch> lockedSoprano;
    std::optional<Pitch> lockedBass;
    int voiceCount{4};
    int maximumLeap{12};
};

struct Observation {
    std::string code;
    std::string fact;
};

struct AttemptResult {
    bool satisfiesConstraints{false};
    std::vector<Observation> observations;
};

[[nodiscard]] inline MovementSize classifyMovement(const int semitones) noexcept {
    const int distance = std::abs(semitones);
    if (distance == 0) return MovementSize::Stationary;
    if (distance == 1) return MovementSize::Semitone;
    if (distance == 2) return MovementSize::WholeStep;
    return MovementSize::Leap;
}

} // namespace vll
