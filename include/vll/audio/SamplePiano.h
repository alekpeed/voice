#pragma once

#include "vll/audio/AudioSample.h"
#include "vll/audio/IInstrument.h"
#include "vll/audio/VelocityCurve.h"

#include <array>
#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace vll::audio {

enum class SampleTrigger { Attack, Release };

struct PianoRegion {
    std::shared_ptr<const AudioSample> sample;
    int lowKey{0};
    int highKey{127};
    int lowVelocity{1};
    int highVelocity{127};
    int rootKey{60};
    float tuneCents{0.0F};
    float gainDecibels{0.0F};
    float pan{-0.0F};
    float releaseSeconds{0.6F};
    SampleTrigger trigger{SampleTrigger::Attack};
};

struct PianoDefinition {
    std::string name;
    std::vector<PianoRegion> regions;
    std::size_t maximumVoices{128};
};

struct PianoLoadResult {
    std::optional<PianoDefinition> definition;
    std::vector<std::string> warnings;
    std::string error;
};

class SfzPianoLoader {
public:
    [[nodiscard]] static PianoLoadResult load(const std::filesystem::path& sfzPath);
};

class SamplePiano final : public IInstrument {
public:
    SamplePiano();

    void prepare(double sampleRate, std::size_t maximumBlockSize) override;
    bool loadPreset(const std::string& presetId) override;
    void noteOn(Pitch pitch, float velocity) noexcept override;
    void noteOff(Pitch pitch) noexcept override;
    void pedal(float amount) noexcept override;
    bool setParameter(const std::string& parameterId, float value) override;
    void renderAudio(StereoBuffer output) noexcept override;
    void allNotesOff() noexcept override;
    [[nodiscard]] int latencySamples() const noexcept override;
    bool savePreset(const std::string& presetName) override;

    bool loadDefinition(PianoDefinition definition);
    void registerPreset(std::string id, std::filesystem::path sfzPath);
    void setVelocityCurve(VelocityCurve curve);
    [[nodiscard]] std::size_t activeVoiceCount() const noexcept;
    [[nodiscard]] const std::string& loadedPresetName() const noexcept;

private:
    enum class CommandType { NoteOn, NoteOff, Pedal, AllNotesOff };
    struct Command { CommandType type; int note; float value; };
    struct Voice {
        const PianoRegion* region{nullptr};
        double position{0.0};
        double increment{1.0};
        float leftGain{1.0F};
        float rightGain{1.0F};
        float envelope{0.0F};
        float attackIncrement{1.0F};
        float releaseMultiplier{1.0F};
        float attackVelocity{0.0F};
        int note{60};
        bool keyReleased{false};
        bool releasing{false};
    };

    static constexpr std::size_t commandCapacity_ = 1024;
    bool enqueue(Command command) noexcept;
    bool dequeue(Command& command) noexcept;
    void applyCommand(const Command& command) noexcept;
    void startRegions(int note, float velocity, SampleTrigger trigger) noexcept;
    void beginRelease(int note) noexcept;
    void beginVoiceRelease(Voice& voice) noexcept;
    void renderVoice(Voice& voice, StereoBuffer output) noexcept;
    void stealVoiceIfNeeded() noexcept;

    PianoDefinition definition_;
    std::unordered_map<std::string, std::filesystem::path> presets_;
    VelocityCurve velocityCurve_;
    std::vector<Voice> voices_;
    std::array<Command, commandCapacity_> commands_{};
    std::atomic<std::size_t> commandWrite_{0};
    std::atomic<std::size_t> commandRead_{0};
    std::atomic<std::size_t> activeVoiceCount_{0};
    std::atomic<bool> panicRequested_{false};
    double sampleRate_{44100.0};
    std::size_t maximumBlockSize_{512};
    float sustainAmount_{0.0F};
    std::atomic<float> masterGain_{1.0F};
};

} // namespace vll::audio
