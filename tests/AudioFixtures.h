#pragma once

#include "vll/audio/AudioSample.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

namespace vll::test {

inline std::shared_ptr<const audio::AudioSample> constantSample(
    const std::size_t frames, const float value, const std::uint32_t sampleRate = 44100) {
    auto sample = std::make_shared<audio::AudioSample>();
    sample->sampleRate = sampleRate;
    sample->left.assign(frames, value);
    sample->right.assign(frames, value);
    return sample;
}

inline void write16(std::ofstream& output, const std::uint16_t value) {
    const std::uint8_t bytes[]{static_cast<std::uint8_t>(value & 0xFFU),
                               static_cast<std::uint8_t>((value >> 8U) & 0xFFU)};
    output.write(reinterpret_cast<const char*>(bytes), 2);
}

inline void write32(std::ofstream& output, const std::uint32_t value) {
    const std::uint8_t bytes[]{static_cast<std::uint8_t>(value & 0xFFU),
                               static_cast<std::uint8_t>((value >> 8U) & 0xFFU),
                               static_cast<std::uint8_t>((value >> 16U) & 0xFFU),
                               static_cast<std::uint8_t>((value >> 24U) & 0xFFU)};
    output.write(reinterpret_cast<const char*>(bytes), 4);
}

inline bool writePcm16Wav(const std::filesystem::path& path,
                          const std::vector<std::int16_t>& interleaved,
                          const std::uint16_t channels = 2,
                          const std::uint32_t sampleRate = 44100) {
    if (channels == 0 || interleaved.size() % channels != 0) return false;
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) return false;
    const auto dataBytes = static_cast<std::uint32_t>(interleaved.size() * sizeof(std::int16_t));
    output.write("RIFF", 4);
    write32(output, 36U + dataBytes);
    output.write("WAVEfmt ", 8);
    write32(output, 16);
    write16(output, 1);
    write16(output, channels);
    write32(output, sampleRate);
    write32(output, sampleRate * channels * 2U);
    write16(output, static_cast<std::uint16_t>(channels * 2U));
    write16(output, 16);
    output.write("data", 4);
    write32(output, dataBytes);
    for (const auto value : interleaved) write16(output, static_cast<std::uint16_t>(value));
    return static_cast<bool>(output);
}

inline float absoluteEnergy(const std::vector<float>& samples) {
    float result = 0.0F;
    for (const float sample : samples) result += std::abs(sample);
    return result;
}

} // namespace vll::test
