#pragma once

#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace vll::audio {

enum class InstrumentFamily { GrandPiano, UprightPiano, FeltPiano, Rhodes, Wurlitzer };
enum class FxStage { Drive, Equalizer, Compressor, Modulation, Amplifier, Reverb };

struct EqBand {
    float frequencyHz{1000.0F};
    float gainDecibels{0.0F};
    float q{0.707F};
    bool bypassed{false};
};

struct InstrumentPreset {
    std::string id;
    std::string name;
    InstrumentFamily family{InstrumentFamily::GrandPiano};
    float tone{0.5F};
    float attackCharacter{0.5F};
    float mechanicalNoise{0.1F};
    float tremoloDepth{0.0F};
    float tremoloRateHz{4.5F};
    float drive{0.0F};
    float reverb{0.15F};
    float globalFineCents{0.0F};
    std::array<float, 128> perNoteCents{};
    std::vector<EqBand> equalizer;
    std::vector<FxStage> fxChain;
};

class InstrumentWorkshop final {
public:
    [[nodiscard]] static std::span<const InstrumentPreset> factoryPresets();
    [[nodiscard]] static std::optional<InstrumentPreset> factoryPreset(const std::string& id);
    [[nodiscard]] static bool validate(const InstrumentPreset& preset);
    [[nodiscard]] static double tuningRatio(const InstrumentPreset& preset, int midiNote);
    [[nodiscard]] static float equalizerGain(const InstrumentPreset& preset, float frequencyHz);
};

} // namespace vll::audio
