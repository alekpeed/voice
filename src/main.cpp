#include "vll/application/AppShell.h"
#include "vll/audio/SamplePiano.h"
#include "vll/audio/LinuxAlsaOutput.h"
#include "vll/core/Logger.h"
#include "vll/core/Settings.h"
#include "vll/midi/LinuxRawMidiInput.h"
#include "vll/midi/MidiSession.h"
#include "vll/midi/VirtualMidiInput.h"

#include <array>
#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>

int main(const int argc, char** argv) {
    const std::string_view command = argc > 1 ? std::string_view(argv[1]) : std::string_view{};
    const bool smokeTest = command == "--smoke-test";
    vll::Logger logger;
    logger.write(vll::LogLevel::Info, "application", "starting Voice Leading Lab shell");

    const auto settings = vll::Settings::defaults();
    vll::application::AppShell shell;
    std::cout << shell.renderTextSnapshot();
    std::cout << "A4 reference: " << settings.value("audio.a4_hz") << " Hz\n";

    if (command == "--list-midi") {
        const vll::midi::LinuxRawMidiInput input;
        const auto devices = input.devices();
        for (const auto& device : devices) std::cout << device.id << " | " << device.name << '\n';
        std::cout << devices.size() << " MIDI input device(s)\n";
    }

    if (command == "--midi-smoke") {
        vll::midi::VirtualMidiInput input;
        vll::midi::MidiSession session(input);
        if (!session.connect(vll::midi::VirtualMidiInput::deviceId())) return 2;
        const std::array<std::uint8_t, 3> noteOn{0x90, 60, 100};
        const std::array<std::uint8_t, 3> noteOff{0x80, 60, 0};
        input.emitBytes(noteOn, 100);
        input.emitBytes(noteOff, 200);
        if (session.recentEvents().size() != 2) return 3;
        std::cout << "MIDI_SMOKE_OK\n";
    }

    if (command == "--audio-smoke") {
        auto sample = std::make_shared<vll::audio::AudioSample>();
        sample->sampleRate = 48000;
        sample->left.resize(4096);
        sample->right.resize(4096);
        for (std::size_t frame = 0; frame < sample->frameCount(); ++frame) {
            const float value = std::exp(-static_cast<float>(frame) / 900.0F) * 0.5F;
            sample->left[frame] = value;
            sample->right[frame] = value;
        }
        vll::audio::PianoDefinition definition;
        definition.name = "Audio smoke fixture";
        definition.regions.push_back({sample});
        vll::audio::SamplePiano piano;
        piano.prepare(48000.0, 256);
        if (!piano.loadDefinition(std::move(definition))) return 4;
        piano.noteOn(vll::Pitch{60}, 0.8F);
        std::vector<float> left(256), right(256);
        piano.renderAudio({left, right});
        float energy = 0.0F;
        for (const float value : left) energy += std::abs(value);
        if (!(energy > 0.0F) || !std::isfinite(energy)) return 5;
        std::cout << "AUDIO_SMOKE_OK\n";
    }

    if (command == "--alsa-null-smoke") {
        auto sample = std::make_shared<vll::audio::AudioSample>();
        sample->sampleRate = 48000;
        sample->left.assign(48000, 0.05F);
        sample->right.assign(48000, 0.05F);
        vll::audio::PianoDefinition definition;
        definition.name = "ALSA null fixture";
        definition.regions.push_back({sample});
        vll::audio::SamplePiano piano;
        if (!piano.loadDefinition(std::move(definition))) return 6;
        vll::audio::LinuxAlsaOutput output;
        if (!output.start(piano, "null", 48000, 128, 5000)) {
            std::cerr << output.lastError() << '\n';
            return 7;
        }
        piano.noteOn(vll::Pitch{60}, 0.5F);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        output.stop();
        std::cout << "ALSA_NULL_SMOKE_OK\n";
    }

    if (smokeTest) std::cout << "SMOKE_TEST_OK\n";
    return 0;
}
