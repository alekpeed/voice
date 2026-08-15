#include "AudioFixtures.h"
#include "TestFramework.h"
#include "vll/audio/AudioSample.h"
#include "vll/audio/SamplePiano.h"

#include <filesystem>
#include <fstream>

TEST_CASE("WAV reader decodes stereo PCM16") {
    const auto directory = std::filesystem::temp_directory_path() / "vll-wav-test";
    std::filesystem::create_directories(directory);
    const auto path = directory / "stereo.wav";
    REQUIRE(vll::test::writePcm16Wav(path, {32767, -32768, 16384, -16384}));
    const auto loaded = vll::audio::WavReader::load(path);
    REQUIRE(loaded.sample.has_value());
    REQUIRE_EQ(loaded.sample->sampleRate, static_cast<std::uint32_t>(44100));
    REQUIRE_EQ(loaded.sample->frameCount(), static_cast<std::size_t>(2));
    REQUIRE(loaded.sample->left[0] > 0.99F);
    REQUIRE(loaded.sample->right[0] <= -0.99F);
    std::filesystem::remove_all(directory);
}

TEST_CASE("SFZ loader resolves inherited regions and quoted sample paths") {
    const auto directory = std::filesystem::temp_directory_path() / "vll-sfz-test";
    const auto samples = directory / "Piano Samples";
    std::filesystem::create_directories(samples);
    REQUIRE(vll::test::writePcm16Wav(samples / "C4 layer.wav", {1000, 1000, 800, 800}));
    const auto sfzPath = directory / "Concert Grand.sfz";
    {
        std::ofstream sfz(sfzPath);
        sfz << "<control> default_path=\"Piano Samples\"\n"
            << "<global> ampeg_release=0.8\n"
            << "<group> lovel=1 hivel=90\n"
            << "<region> sample=\"C4 layer.wav\" key=60\n";
    }
    const auto loaded = vll::audio::SfzPianoLoader::load(sfzPath);
    REQUIRE(loaded.definition.has_value());
    REQUIRE_EQ(loaded.definition->regions.size(), static_cast<std::size_t>(1));
    REQUIRE_EQ(loaded.definition->regions[0].rootKey, 60);
    REQUIRE_EQ(loaded.definition->regions[0].highVelocity, 90);
    REQUIRE(loaded.definition->regions[0].sample->valid());
    std::filesystem::remove_all(directory);
}

TEST_CASE("WAV reader rejects unsupported content") {
    const auto path = std::filesystem::temp_directory_path() / "vll-invalid.wav";
    {
        std::ofstream output(path, std::ios::binary);
        output << "not a wave file";
    }
    const auto loaded = vll::audio::WavReader::load(path);
    REQUIRE(!loaded.sample.has_value());
    REQUIRE(!loaded.error.empty());
    std::filesystem::remove(path);
}
