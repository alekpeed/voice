#include "AudioFixtures.h"
#include "TestFramework.h"
#include "vll/audio/MidiAudioRouter.h"
#include "vll/audio/SamplePiano.h"

#include <vector>

namespace {

vll::audio::PianoDefinition oneRegionPiano(const std::size_t frames = 30000) {
    vll::audio::PianoDefinition definition;
    definition.name = "Test Grand";
    definition.maximumVoices = 16;
    vll::audio::PianoRegion region;
    region.sample = vll::test::constantSample(frames, 0.5F);
    region.releaseSeconds = 0.01F;
    definition.regions.push_back(region);
    return definition;
}

void renderBlocks(vll::audio::SamplePiano& piano, const int blocks,
                  const std::size_t blockSize = 256) {
    std::vector<float> left(blockSize);
    std::vector<float> right(blockSize);
    for (int block = 0; block < blocks; ++block) piano.renderAudio({left, right});
}

} // namespace

TEST_CASE("sample piano renders a pitched attack") {
    vll::audio::SamplePiano piano;
    piano.prepare(44100.0, 256);
    REQUIRE(piano.loadDefinition(oneRegionPiano()));
    piano.noteOn(vll::Pitch{60}, 0.8F);
    std::vector<float> left(256);
    std::vector<float> right(256);
    piano.renderAudio({left, right});
    REQUIRE(vll::test::absoluteEnergy(left) > 1.0F);
    REQUIRE_EQ(piano.activeVoiceCount(), static_cast<std::size_t>(1));
    REQUIRE_EQ(piano.loadedPresetName(), std::string("Test Grand"));
}

TEST_CASE("sustain holds a released note until pedal-up") {
    vll::audio::SamplePiano piano;
    piano.prepare(44100.0, 256);
    REQUIRE(piano.loadDefinition(oneRegionPiano()));
    piano.noteOn(vll::Pitch{60}, 0.8F);
    renderBlocks(piano, 1);
    piano.pedal(1.0F);
    piano.noteOff(vll::Pitch{60});
    renderBlocks(piano, 20);
    REQUIRE_EQ(piano.activeVoiceCount(), static_cast<std::size_t>(1));
    piano.pedal(0.0F);
    renderBlocks(piano, 12);
    REQUIRE_EQ(piano.activeVoiceCount(), static_cast<std::size_t>(0));
}

TEST_CASE("polyphony limit steals a quiet or releasing voice") {
    auto definition = oneRegionPiano();
    definition.maximumVoices = 2;
    vll::audio::SamplePiano piano;
    piano.prepare(44100.0, 64);
    REQUIRE(piano.loadDefinition(std::move(definition)));
    piano.noteOn(vll::Pitch{60}, 0.3F);
    piano.noteOn(vll::Pitch{64}, 0.5F);
    piano.noteOn(vll::Pitch{67}, 0.8F);
    renderBlocks(piano, 1, 64);
    REQUIRE_EQ(piano.activeVoiceCount(), static_cast<std::size_t>(2));
}

TEST_CASE("velocity layers select different sample regions") {
    vll::audio::PianoDefinition definition;
    definition.name = "Layered";
    vll::audio::PianoRegion soft;
    soft.sample = vll::test::constantSample(2000, 0.1F);
    soft.highVelocity = 63;
    vll::audio::PianoRegion loud;
    loud.sample = vll::test::constantSample(2000, 0.8F);
    loud.lowVelocity = 64;
    definition.regions = {soft, loud};

    vll::audio::SamplePiano softPiano;
    softPiano.prepare(44100.0, 256);
    REQUIRE(softPiano.loadDefinition(definition));
    softPiano.noteOn(vll::Pitch{60}, 0.4F);
    std::vector<float> softLeft(256), softRight(256);
    softPiano.renderAudio({softLeft, softRight});

    vll::audio::SamplePiano loudPiano;
    loudPiano.prepare(44100.0, 256);
    REQUIRE(loudPiano.loadDefinition(std::move(definition)));
    loudPiano.noteOn(vll::Pitch{60}, 0.8F);
    std::vector<float> loudLeft(256), loudRight(256);
    loudPiano.renderAudio({loudLeft, loudRight});
    REQUIRE(vll::test::absoluteEnergy(loudLeft) > vll::test::absoluteEnergy(softLeft) * 4.0F);
}

TEST_CASE("MIDI router sends notes and pedal to the instrument") {
    vll::audio::SamplePiano piano;
    piano.prepare(44100.0, 128);
    REQUIRE(piano.loadDefinition(oneRegionPiano()));
    vll::audio::MidiAudioRouter router(piano);
    router.process({vll::NoteEventType::NoteOn, vll::Pitch{60}, 0.7F, 1, 1});
    router.process({vll::NoteEventType::SustainOn, vll::Pitch{0}, 1.0F, 2, 1});
    renderBlocks(piano, 1, 128);
    REQUIRE_EQ(piano.activeVoiceCount(), static_cast<std::size_t>(1));
    router.process({vll::NoteEventType::NoteOff, vll::Pitch{60}, 0.0F, 3, 1});
    router.process({vll::NoteEventType::SustainOff, vll::Pitch{0}, 0.0F, 4, 1});
    renderBlocks(piano, 20, 128);
    REQUIRE_EQ(piano.activeVoiceCount(), static_cast<std::size_t>(0));
}

TEST_CASE("command overflow requests a safe all-notes-off") {
    vll::audio::SamplePiano piano;
    piano.prepare(44100.0, 128);
    REQUIRE(piano.loadDefinition(oneRegionPiano(100000)));
    for (int event = 0; event < 1200; ++event) piano.noteOn(vll::Pitch{60 + event % 12}, 0.5F);
    renderBlocks(piano, 1, 128);
    renderBlocks(piano, 60, 128);
    REQUIRE_EQ(piano.activeVoiceCount(), static_cast<std::size_t>(0));
}

TEST_CASE("extended dense rendering remains finite and bounded") {
    auto definition = oneRegionPiano(500000);
    definition.maximumVoices = 32;
    vll::audio::SamplePiano piano;
    piano.prepare(44100.0, 256);
    REQUIRE(piano.loadDefinition(std::move(definition)));
    std::vector<float> left(256), right(256);
    for (int block = 0; block < 3000; ++block) {
        if (block % 24 == 0) piano.noteOn(vll::Pitch{48 + (block / 24) % 36}, 0.7F);
        if (block % 24 == 12) piano.noteOff(vll::Pitch{48 + (block / 24) % 36});
        piano.renderAudio({left, right});
        for (const float value : left) REQUIRE(std::isfinite(value));
        REQUIRE(piano.activeVoiceCount() <= static_cast<std::size_t>(32));
    }
}
