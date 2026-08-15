#include "TestFramework.h"
#include "vll/midi/MidiSession.h"
#include "vll/midi/VirtualMidiInput.h"

#include <array>

TEST_CASE("virtual input enumerates and requires a valid connection") {
    vll::midi::VirtualMidiInput input;
    REQUIRE_EQ(input.devices().size(), static_cast<std::size_t>(1));
    REQUIRE(!input.connect("missing"));
    REQUIRE(input.connect(vll::midi::VirtualMidiInput::deviceId()));
    REQUIRE(input.isConnected());
}

TEST_CASE("session prevents stuck notes when a device disconnects") {
    vll::midi::VirtualMidiInput input;
    vll::midi::MidiSession session(input);
    REQUIRE(session.connect(vll::midi::VirtualMidiInput::deviceId()));

    const std::array<std::uint8_t, 3> noteOn{0x90, 60, 100};
    const std::array<std::uint8_t, 3> pedalDown{0xB0, 64, 127};
    input.emitBytes(noteOn, 100);
    input.emitBytes(pedalDown, 110);
    REQUIRE_EQ(session.activeNotes().size(), static_cast<std::size_t>(1));
    REQUIRE(session.sustainDown());

    session.disconnect();
    REQUIRE(session.activeNotes().empty());
    REQUIRE(!session.sustainDown());
    const auto events = session.recentEvents();
    REQUIRE_EQ(events.size(), static_cast<std::size_t>(4));
    REQUIRE(events[2].synthetic);
    REQUIRE_EQ(events[2].event.type, vll::NoteEventType::NoteOff);
    REQUIRE(events[3].synthetic);
    REQUIRE_EQ(events[3].event.type, vll::NoteEventType::SustainOff);
}

TEST_CASE("retrigger inserts a note-off before the second attack") {
    vll::midi::VirtualMidiInput input;
    vll::midi::MidiSession session(input);
    session.connect(vll::midi::VirtualMidiInput::deviceId());

    const std::array<std::uint8_t, 3> attack{0x90, 72, 90};
    input.emitBytes(attack, 10);
    input.emitBytes(attack, 20);
    const auto events = session.recentEvents();
    REQUIRE_EQ(events.size(), static_cast<std::size_t>(3));
    REQUIRE_EQ(events[1].event.type, vll::NoteEventType::NoteOff);
    REQUIRE(events[1].synthetic);
    REQUIRE_EQ(events[2].event.type, vll::NoteEventType::NoteOn);
}

TEST_CASE("unexpected device loss also prevents stuck notes") {
    vll::midi::VirtualMidiInput input;
    vll::midi::MidiSession session(input);
    session.connect(vll::midi::VirtualMidiInput::deviceId());
    const std::array<std::uint8_t, 3> noteOn{0x90, 55, 100};
    input.emitBytes(noteOn, 25);
    input.simulateConnectionLoss();

    REQUIRE(!session.isConnected());
    REQUIRE(session.activeNotes().empty());
    const auto events = session.recentEvents();
    REQUIRE_EQ(events.size(), static_cast<std::size_t>(2));
    REQUIRE(events.back().synthetic);
    REQUIRE_EQ(events.back().event.type, vll::NoteEventType::NoteOff);
}

TEST_CASE("timestamps remain monotonic across faulty device timestamps") {
    vll::midi::VirtualMidiInput input;
    vll::midi::MidiSession session(input);
    session.connect(vll::midi::VirtualMidiInput::deviceId());
    const std::array<std::uint8_t, 3> first{0x90, 60, 80};
    const std::array<std::uint8_t, 3> second{0x90, 64, 80};
    input.emitBytes(first, 200);
    input.emitBytes(second, 100);
    const auto events = session.recentEvents();
    REQUIRE_EQ(events[0].event.timestamp, static_cast<vll::TimestampMicros>(200));
    REQUIRE_EQ(events[1].event.timestamp, static_cast<vll::TimestampMicros>(200));
}

TEST_CASE("MIDI fixtures replay deterministically") {
    vll::midi::VirtualMidiInput input;
    vll::midi::MidiSession session(input);
    session.connect(vll::midi::VirtualMidiInput::deviceId());
    const std::vector<vll::midi::MidiFixturePacket> fixture{
        {100, {0x90, 60, 64}},
        {200, {0x80, 60, 0}},
        {300, {0xB0, 64, 127}},
        {400, {0xB0, 64, 0}}
    };
    input.playFixture(fixture);
    const auto first = session.recentEvents();

    session.disconnect();
    REQUIRE(session.connect(vll::midi::VirtualMidiInput::deviceId()));
    input.playFixture(fixture);
    const auto second = session.recentEvents();
    REQUIRE_EQ(first.size(), static_cast<std::size_t>(4));
    REQUIRE_EQ(second.size(), static_cast<std::size_t>(8));
    for (std::size_t index = 0; index < first.size(); ++index) {
        REQUIRE_EQ(first[index].event.type, second[index + 4].event.type);
        REQUIRE_EQ(first[index].event.timestamp, second[index + 4].event.timestamp);
    }
}
