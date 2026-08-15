#include "vll/audio/InstrumentWorkshop.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace vll::audio {
namespace {

InstrumentPreset preset(std::string id, std::string name, InstrumentFamily family,
                        float tone, float attack, float noise, float tremolo,
                        float drive, float reverb, std::vector<EqBand> eq,
                        std::vector<FxStage> fx) {
    InstrumentPreset result;
    result.id = std::move(id);
    result.name = std::move(name);
    result.family = family;
    result.tone = tone;
    result.attackCharacter = attack;
    result.mechanicalNoise = noise;
    result.tremoloDepth = tremolo;
    result.drive = drive;
    result.reverb = reverb;
    result.equalizer = std::move(eq);
    result.fxChain = std::move(fx);
    return result;
}

const std::array<InstrumentPreset, 7> kPresets{
    preset("grand-natural", "Concert Grand Natural", InstrumentFamily::GrandPiano,
           0.55F, 0.55F, 0.08F, 0.0F, 0.0F, 0.18F,
           {{90.0F, 1.0F, 0.7F, false}, {2500.0F, 0.5F, 1.0F, false}},
           {FxStage::Equalizer, FxStage::Reverb}),
    preset("studio-grand-dry", "Studio Grand Dry", InstrumentFamily::GrandPiano,
           0.6F, 0.65F, 0.05F, 0.0F, 0.0F, 0.04F,
           {{180.0F, -1.0F, 0.8F, false}}, {FxStage::Equalizer}),
    preset("upright-warm", "Upright Warm", InstrumentFamily::UprightPiano,
           0.38F, 0.48F, 0.18F, 0.0F, 0.03F, 0.12F,
           {{220.0F, 1.8F, 0.9F, false}, {3200.0F, -1.5F, 0.8F, false}},
           {FxStage::Drive, FxStage::Equalizer, FxStage::Reverb}),
    preset("felt-intimate", "Felt Intimate", InstrumentFamily::FeltPiano,
           0.24F, 0.2F, 0.22F, 0.0F, 0.0F, 0.25F,
           {{2500.0F, -4.0F, 0.7F, false}}, {FxStage::Equalizer, FxStage::Reverb}),
    preset("rhodes-suitcase", "Rhodes Suitcase", InstrumentFamily::Rhodes,
           0.58F, 0.62F, 0.12F, 0.32F, 0.12F, 0.2F,
           {{160.0F, 1.0F, 0.8F, false}, {3200.0F, 2.2F, 1.1F, false}},
           {FxStage::Drive, FxStage::Equalizer, FxStage::Compressor,
            FxStage::Modulation, FxStage::Amplifier, FxStage::Reverb}),
    preset("rhodes-bark", "Rhodes Stage Bark", InstrumentFamily::Rhodes,
           0.7F, 0.82F, 0.14F, 0.08F, 0.28F, 0.12F,
           {{700.0F, -1.0F, 1.0F, false}, {4100.0F, 3.0F, 1.2F, false}},
           {FxStage::Drive, FxStage::Equalizer, FxStage::Compressor,
            FxStage::Amplifier, FxStage::Reverb}),
    preset("wurlitzer-200a", "Wurlitzer 200A", InstrumentFamily::Wurlitzer,
           0.62F, 0.75F, 0.2F, 0.2F, 0.22F, 0.14F,
           {{180.0F, -1.5F, 0.8F, false}, {1900.0F, 2.5F, 1.0F, false}},
           {FxStage::Drive, FxStage::Equalizer, FxStage::Compressor,
            FxStage::Modulation, FxStage::Amplifier, FxStage::Reverb}),
};

bool unitValue(const float value) { return value >= 0.0F && value <= 1.0F; }

} // namespace

std::span<const InstrumentPreset> InstrumentWorkshop::factoryPresets() { return kPresets; }

std::optional<InstrumentPreset> InstrumentWorkshop::factoryPreset(const std::string& id) {
    const auto found = std::ranges::find(kPresets, id, &InstrumentPreset::id);
    return found == kPresets.end() ? std::nullopt : std::optional<InstrumentPreset>{*found};
}

bool InstrumentWorkshop::validate(const InstrumentPreset& item) {
    if (item.id.empty() || item.name.empty() || !unitValue(item.tone) ||
        !unitValue(item.attackCharacter) || !unitValue(item.mechanicalNoise) ||
        !unitValue(item.tremoloDepth) || item.tremoloRateHz < 0.1F ||
        item.tremoloRateHz > 20.0F || !unitValue(item.drive) || !unitValue(item.reverb) ||
        std::abs(item.globalFineCents) > 100.0F || item.fxChain.empty()) return false;
    return std::ranges::all_of(item.equalizer, [](const EqBand& band) {
        return band.frequencyHz >= 20.0F && band.frequencyHz <= 20000.0F &&
               band.gainDecibels >= -24.0F && band.gainDecibels <= 24.0F &&
               band.q >= 0.1F && band.q <= 20.0F;
    }) && std::ranges::all_of(item.perNoteCents, [](const float cents) {
        return std::abs(cents) <= 100.0F;
    });
}

double InstrumentWorkshop::tuningRatio(const InstrumentPreset& item, const int midiNote) {
    if (midiNote < 0 || midiNote > 127) return 1.0;
    const double cents = static_cast<double>(item.globalFineCents + item.perNoteCents[midiNote]);
    return std::pow(2.0, cents / 1200.0);
}

float InstrumentWorkshop::equalizerGain(const InstrumentPreset& item, const float frequencyHz) {
    if (frequencyHz <= 0.0F) return 1.0F;
    double decibels = 0.0;
    for (const auto& band : item.equalizer) {
        if (band.bypassed) continue;
        const double octaves = std::log2(static_cast<double>(frequencyHz / band.frequencyHz));
        const double width = 1.0 / static_cast<double>(band.q);
        decibels += static_cast<double>(band.gainDecibels) *
                    std::exp(-0.5 * (octaves / width) * (octaves / width));
    }
    return static_cast<float>(std::pow(10.0, decibels / 20.0));
}

} // namespace vll::audio
