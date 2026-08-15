#pragma once

#include "vll/core/Types.h"
#include <cstddef>
#include <span>
#include <string>

namespace vll::audio {

struct StereoBuffer {
    std::span<float> left;
    std::span<float> right;
};

class IInstrument {
public:
    virtual ~IInstrument() = default;
    virtual void prepare(double sampleRate, std::size_t maximumBlockSize) = 0;
    virtual bool loadPreset(const std::string& presetId) = 0;
    virtual void noteOn(Pitch pitch, float velocity) noexcept = 0;
    virtual void noteOff(Pitch pitch) noexcept = 0;
    virtual void pedal(float amount) noexcept = 0;
    virtual bool setParameter(const std::string& parameterId, float value) = 0;
    virtual void renderAudio(StereoBuffer output) noexcept = 0;
    virtual void allNotesOff() noexcept = 0;
    [[nodiscard]] virtual int latencySamples() const noexcept = 0;
    virtual bool savePreset(const std::string& presetName) = 0;
};

} // namespace vll::audio
