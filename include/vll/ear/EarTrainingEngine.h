#pragma once
#include "vll/core/Types.h"
#include <cstdint>
#include <string>
#include <vector>
namespace vll::ear {
enum class Task { Direction, CommonTone, IsolatedVoice, Reconstruction };
struct EarPrompt { std::string id; std::uint64_t seed{0}; Task task{Task::Direction}; std::vector<Sonority> progression; VoiceId targetVoice{1}; int correctAnswer{0}; };
struct EarResult { bool correct{false}; Observation fact; };
class EarTrainingEngine final { public: [[nodiscard]] EarPrompt generate(std::uint64_t seed,Task task) const; [[nodiscard]] EarResult submit(const EarPrompt&,int answer) const; [[nodiscard]] std::vector<NoteEvent> isolatedPlayback(const EarPrompt&) const; };
}
