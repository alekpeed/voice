#include "vll/application/AppShell.h"
#include "vll/core/Logger.h"
#include "vll/core/Settings.h"
#include "vll/midi/LinuxRawMidiInput.h"
#include "vll/midi/MidiSession.h"
#include "vll/midi/VirtualMidiInput.h"

#include <array>
#include <iostream>
#include <string_view>

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

    if (smokeTest) std::cout << "SMOKE_TEST_OK\n";
    return 0;
}
