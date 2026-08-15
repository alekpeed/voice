#include "vll/application/AppShell.h"
#include "vll/analysis/VoiceAssigner.h"
#include "vll/audio/SamplePiano.h"
#include "vll/audio/LinuxAlsaOutput.h"
#include "vll/audio/MidiAudioRouter.h"
#include "vll/core/Logger.h"
#include "vll/core/Settings.h"
#include "vll/midi/LinuxRawMidiInput.h"
#include "vll/midi/MidiSession.h"
#include "vll/midi/VirtualMidiInput.h"
#include "vll/notation/SvgNotationRenderer.h"
#include "vll/visualization/VisualizationModel.h"
#include "vll/visualization/VoiceGraphSvgRenderer.h"
#include "vll/visualization/VoicePathBuilder.h"

#include <array>
#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>
#include <memory>
#include <filesystem>
#include <string_view>
#include <thread>
#include <vector>

namespace {

std::string createVisualizerFixtureSvg() {
    const std::vector<vll::Sonority> progression{
        {{{50}, {53}, {60}, {64}}, 0, 80'000},
        {{{43}, {53}, {59}, {62}}, 1'000'000, 1'080'000},
        {{{48}, {52}, {59}, {62}}, 2'000'000, 2'080'000},
    };
    vll::ExerciseConstraint constraints;
    constraints.voiceCount = 4;
    constraints.maximumLeap = 12;
    const vll::analysis::VoiceAssigner assigner;
    const auto built = vll::visualization::VoicePathBuilder(assigner).build(
        progression, constraints);
    if (!built.complete) return {};

    vll::visualization::VisualizationModel model(built.paths);
    model.setTimeline({{0, "ii"}, {1'000'000, "V"}, {2'000'000, "I"}});
    model.setCursor(1'000'000);
    model.setPlaybackRate(0.5);
    return vll::visualization::VoiceGraphSvgRenderer{}.render(model.frame());
}

std::string createNotationFixtureSvg() {
    const vll::notation::NotationDocument document{
        {
            {0, {{{50}, 1, 5, false}, {{53}, 2, 3, false},
                 {{60}, 3, 2, true}, {{64}, 4, 1, false}}, "Dm9", 1.0},
            {1'000'000, {{{43}, 1, 5, false}, {{53}, 2, 3, false},
                         {{59}, 3, 2, true}, {{64}, 4, 1, false}}, "G13", 1.0},
            {2'000'000, {{{48}, 1, 5, false}, {{52}, 2, 3, false},
                         {{59}, 3, 2, true}, {{62}, 4, 1, false}}, "Cmaj9", 2.0},
        },
        500'000,
    };
    vll::notation::EngravingOptions options;
    options.showAnalysisMarks = false;
    options.showFingering = false;
    options.highlightedVoice = 3;
    return vll::notation::SvgNotationRenderer{}.render(document, options).scalableVectorData;
}

} // namespace

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

    if (command == "--play-piano") {
        if (argc < 3) {
            std::cerr << "Usage: voice-leading-lab --play-piano <piano.sfz> [midi-device] [alsa-device]\n";
            return 8;
        }
        auto loaded = vll::audio::SfzPianoLoader::load(std::filesystem::path(argv[2]));
        if (!loaded.definition) {
            std::cerr << loaded.error << '\n';
            return 9;
        }
        for (const auto& warning : loaded.warnings) std::cerr << "Sample warning: " << warning << '\n';
        vll::audio::SamplePiano piano;
        if (!piano.loadDefinition(std::move(*loaded.definition))) return 10;

        vll::midi::LinuxRawMidiInput midiInput;
        const auto midiDevices = midiInput.devices();
        if (midiDevices.empty()) {
            std::cerr << "No Linux raw-MIDI input devices found\n";
            return 11;
        }
        const std::string midiDevice = argc >= 4 ? argv[3] : midiDevices.front().id;
        const std::string audioDevice = argc >= 5 ? argv[4] : "default";

        vll::audio::LinuxAlsaOutput audioOutput;
        if (!audioOutput.start(piano, audioDevice, 48000, 128, 10000)) {
            std::cerr << audioOutput.lastError() << '\n';
            return 12;
        }
        vll::audio::MidiAudioRouter router(piano);
        vll::midi::MidiSession midiSession(midiInput);
        midiSession.setEventHandler([&router](const vll::midi::MonitoredEvent& event) {
            router.process(event.event);
        });
        if (!midiSession.connect(midiDevice)) {
            std::cerr << "Unable to connect MIDI input: " << midiDevice << '\n';
            audioOutput.stop();
            return 13;
        }
        std::cout << "Playing " << piano.loadedPresetName() << " from " << midiDevice
                  << " through ALSA " << audioDevice << ". Press Enter to stop.\n";
        std::string ignored;
        std::getline(std::cin, ignored);
        midiSession.disconnect();
        piano.allNotesOff();
        audioOutput.stop();
    }

    if (command == "--visualizer-smoke" || command == "--export-visualizer-svg") {
        const auto svg = createVisualizerFixtureSvg();
        if (svg.find("Voice-leading graph") == std::string::npos ||
            svg.find(">ii</text>") == std::string::npos ||
            svg.find("-1 semitone") == std::string::npos) {
            return 14;
        }
        if (command == "--export-visualizer-svg") {
            if (argc < 3) {
                std::cerr << "Usage: voice-leading-lab --export-visualizer-svg <output.svg>\n";
                return 15;
            }
            std::ofstream output(argv[2], std::ios::binary | std::ios::trunc);
            output << svg;
            if (!output) return 16;
            std::cout << "Visualizer SVG written to " << argv[2] << '\n';
        } else {
            std::cout << "VISUALIZER_SMOKE_OK\n";
        }
    }

    if (command == "--notation-smoke" || command == "--export-notation-svg") {
        const auto svg = createNotationFixtureSvg();
        if (svg.find("Grand-staff notation") == std::string::npos ||
            svg.find("Dm9") == std::string::npos ||
            svg.find("data-role=\"ledger-line\"") == std::string::npos ||
            svg.find("data-role=\"voice-highlight\"") == std::string::npos ||
            svg.find("data-role=\"playback-cursor\"") == std::string::npos) {
            return 17;
        }
        if (command == "--export-notation-svg") {
            if (argc < 3) {
                std::cerr << "Usage: voice-leading-lab --export-notation-svg <output.svg>\n";
                return 18;
            }
            std::ofstream output(argv[2], std::ios::binary | std::ios::trunc);
            output << svg;
            if (!output) return 19;
            std::cout << "Notation SVG written to " << argv[2] << '\n';
        } else {
            std::cout << "NOTATION_SMOKE_OK\n";
        }
    }

    if (smokeTest) std::cout << "SMOKE_TEST_OK\n";
    return 0;
}
