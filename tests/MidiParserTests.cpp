#include "TestFramework.h"
#include "vll/midi/MidiByteStreamParser.h"

TEST_CASE("MIDI parser emits notes with velocity and channel") {
    vll::midi::MidiByteStreamParser parser;
    REQUIRE(!parser.feed(0x92, 100).has_value());
    REQUIRE(!parser.feed(60, 100).has_value());
    const auto event = parser.feed(96, 100);
    REQUIRE(event.has_value());
    REQUIRE_EQ(event->type, vll::NoteEventType::NoteOn);
    REQUIRE_EQ(event->pitch.midiNote, 60);
    REQUIRE_EQ(event->channel, 3);
    REQUIRE(event->velocity > 0.75F);
}

TEST_CASE("zero velocity note-on is canonical note-off") {
    vll::midi::MidiByteStreamParser parser;
    REQUIRE(!parser.feed(0x90, 10).has_value());
    REQUIRE(!parser.feed(64, 10).has_value());
    const auto event = parser.feed(0, 10);
    REQUIRE(event.has_value());
    REQUIRE_EQ(event->type, vll::NoteEventType::NoteOff);
}

TEST_CASE("MIDI running status and sustain are parsed") {
    vll::midi::MidiByteStreamParser parser;
    REQUIRE(!parser.feed(0xB0, 1).has_value());
    REQUIRE(!parser.feed(64, 1).has_value());
    const auto down = parser.feed(127, 1);
    REQUIRE(down.has_value());
    REQUIRE_EQ(down->type, vll::NoteEventType::SustainOn);

    REQUIRE(!parser.feed(64, 2).has_value());
    const auto up = parser.feed(0, 2);
    REQUIRE(up.has_value());
    REQUIRE_EQ(up->type, vll::NoteEventType::SustainOff);
}

TEST_CASE("real-time bytes do not disrupt a channel message") {
    vll::midi::MidiByteStreamParser parser;
    REQUIRE(!parser.feed(0x90, 1).has_value());
    REQUIRE(!parser.feed(67, 1).has_value());
    REQUIRE(!parser.feed(0xF8, 1).has_value());
    const auto event = parser.feed(80, 1);
    REQUIRE(event.has_value());
    REQUIRE_EQ(event->pitch.midiNote, 67);
}
