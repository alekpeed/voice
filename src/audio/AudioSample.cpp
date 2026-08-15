#include "vll/audio/AudioSample.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <limits>

namespace vll::audio {
namespace {

std::uint16_t little16(const std::uint8_t* data) noexcept {
    return static_cast<std::uint16_t>(data[0]) |
           static_cast<std::uint16_t>(data[1] << 8U);
}

std::uint32_t little32(const std::uint8_t* data) noexcept {
    return static_cast<std::uint32_t>(data[0]) |
           (static_cast<std::uint32_t>(data[1]) << 8U) |
           (static_cast<std::uint32_t>(data[2]) << 16U) |
           (static_cast<std::uint32_t>(data[3]) << 24U);
}

bool tagEquals(const std::uint8_t* data, const char* tag) noexcept {
    return std::memcmp(data, tag, 4) == 0;
}

float decodePcm(const std::uint8_t* data, const int bits) noexcept {
    if (bits == 16) {
        const auto value = static_cast<std::int16_t>(little16(data));
        return static_cast<float>(value) / 32768.0F;
    }
    if (bits == 24) {
        std::int32_t value = static_cast<std::int32_t>(data[0]) |
                             (static_cast<std::int32_t>(data[1]) << 8) |
                             (static_cast<std::int32_t>(data[2]) << 16);
        if ((value & 0x00800000) != 0) value |= static_cast<std::int32_t>(0xFF000000);
        return static_cast<float>(value) / 8388608.0F;
    }
    const auto value = static_cast<std::int32_t>(little32(data));
    return static_cast<float>(static_cast<double>(value) / 2147483648.0);
}

} // namespace

SampleLoadResult WavReader::load(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) return {std::nullopt, "Unable to open WAV file: " + path.string()};
    const auto fileSize = input.tellg();
    if (fileSize < 12) return {std::nullopt, "WAV file is too short"};
    input.seekg(0);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(fileSize));
    input.read(reinterpret_cast<char*>(bytes.data()), fileSize);
    if (!input) return {std::nullopt, "Unable to read complete WAV file"};

    if (!tagEquals(bytes.data(), "RIFF") || !tagEquals(bytes.data() + 8, "WAVE")) {
        return {std::nullopt, "Only little-endian RIFF/WAVE files are supported"};
    }

    std::uint16_t format = 0;
    std::uint16_t channels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bits = 0;
    const std::uint8_t* sampleBytes = nullptr;
    std::size_t sampleByteCount = 0;

    std::size_t offset = 12;
    while (offset + 8 <= bytes.size()) {
        const auto chunkSize = static_cast<std::size_t>(little32(bytes.data() + offset + 4));
        const auto payload = offset + 8;
        if (payload > bytes.size() || chunkSize > bytes.size() - payload) {
            return {std::nullopt, "WAV chunk extends past end of file"};
        }
        if (tagEquals(bytes.data() + offset, "fmt ") && chunkSize >= 16) {
            format = little16(bytes.data() + payload);
            channels = little16(bytes.data() + payload + 2);
            sampleRate = little32(bytes.data() + payload + 4);
            bits = little16(bytes.data() + payload + 14);
        } else if (tagEquals(bytes.data() + offset, "data")) {
            sampleBytes = bytes.data() + payload;
            sampleByteCount = chunkSize;
        }
        offset = payload + chunkSize + (chunkSize & 1U);
    }

    if (sampleBytes == nullptr || format == 0) return {std::nullopt, "WAV is missing fmt or data"};
    if (channels != 1 && channels != 2) return {std::nullopt, "Only mono and stereo WAV files are supported"};
    if (sampleRate == 0) return {std::nullopt, "WAV sample rate is invalid"};
    if ((format != 1 || (bits != 16 && bits != 24 && bits != 32)) &&
        (format != 3 || bits != 32)) {
        return {std::nullopt, "Supported WAV encodings are PCM 16/24/32 and float 32"};
    }

    const std::size_t bytesPerSample = bits / 8U;
    const std::size_t bytesPerFrame = bytesPerSample * channels;
    if (bytesPerFrame == 0 || sampleByteCount % bytesPerFrame != 0) {
        return {std::nullopt, "WAV data does not contain complete frames"};
    }

    AudioSample sample;
    sample.sampleRate = sampleRate;
    const std::size_t frames = sampleByteCount / bytesPerFrame;
    sample.left.resize(frames);
    sample.right.resize(frames);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        const auto* frameData = sampleBytes + frame * bytesPerFrame;
        auto decode = [&](const std::uint8_t* value) {
            if (format == 1) return decodePcm(value, bits);
            float result = 0.0F;
            std::memcpy(&result, value, sizeof(float));
            return std::clamp(result, -1.0F, 1.0F);
        };
        sample.left[frame] = decode(frameData);
        sample.right[frame] = channels == 2 ? decode(frameData + bytesPerSample) : sample.left[frame];
    }
    return {std::move(sample), {}};
}

} // namespace vll::audio
