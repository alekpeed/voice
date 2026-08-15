#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace vll::audio {

struct AudioSample {
    std::uint32_t sampleRate{44100};
    std::vector<float> left;
    std::vector<float> right;

    [[nodiscard]] std::size_t frameCount() const noexcept { return left.size(); }
    [[nodiscard]] bool valid() const noexcept {
        return sampleRate > 0 && !left.empty() && left.size() == right.size();
    }
};

struct SampleLoadResult {
    std::optional<AudioSample> sample;
    std::string error;
};

class WavReader {
public:
    [[nodiscard]] static SampleLoadResult load(const std::filesystem::path& path);
};

} // namespace vll::audio
