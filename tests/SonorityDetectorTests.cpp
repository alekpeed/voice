#include "TestFramework.h"
#include "vll/performance/SonorityDetector.h"

#include <cstddef>

namespace {

vll::NoteEvent note(const vll::NoteEventType type, const int midiNote,
                    const vll::TimestampMicros timestamp, const int channel = 1) {
    return {type, vll::Pitch{midiNote}, type == vll::NoteEventType::NoteOn ? 0.8F : 0.0F,
            timestamp, channel};
}

} // namespace

TEST_CASE("rolled attacks inside the chord window form one sonority") {
    vll::performance::SonorityDetector detector({80'000, 2, 16});
    detector.ingest(note(vll::NoteEventType::NoteOn, 60, 0));
    detector.ingest(note(vll::NoteEventType::NoteOn, 64, 30'000));
    detector.ingest(note(vll::NoteEventType::NoteOn, 67, 70'000));
    detector.advanceTime(79'999);
    REQUIRE(detector.completedSonorities().empty());

    detector.advanceTime(80'000);
    const auto sonorities = detector.completedSonorities();
    REQUIRE_EQ(sonorities.size(), std::size_t{1});
    REQUIRE_EQ(sonorities[0].pitches.size(), std::size_t{3});
    REQUIRE_EQ(sonorities[0].pitches[0], vll::Pitch{60});
    REQUIRE_EQ(sonorities[0].pitches[1], vll::Pitch{64});
    REQUIRE_EQ(sonorities[0].pitches[2], vll::Pitch{67});
    REQUIRE_EQ(sonorities[0].startedAt, vll::TimestampMicros{0});
    REQUIRE_EQ(sonorities[0].endedAt, vll::TimestampMicros{70'000});
}

TEST_CASE("legato releases inside a chord window produce the destination sonority") {
    vll::performance::SonorityDetector detector({80'000, 2, 16});
    detector.ingest(note(vll::NoteEventType::NoteOn, 60, 0));
    detector.ingest(note(vll::NoteEventType::NoteOn, 64, 10'000));
    detector.ingest(note(vll::NoteEventType::NoteOn, 67, 20'000));
    detector.advanceTime(80'000);

    detector.ingest(note(vll::NoteEventType::NoteOn, 65, 200'000));
    detector.ingest(note(vll::NoteEventType::NoteOff, 64, 210'000));
    detector.ingest(note(vll::NoteEventType::NoteOn, 69, 220'000));
    detector.ingest(note(vll::NoteEventType::NoteOff, 67, 230'000));
    detector.advanceTime(280'000);

    const auto sonorities = detector.completedSonorities();
    REQUIRE_EQ(sonorities.size(), std::size_t{2});
    REQUIRE_EQ(sonorities[1].pitches.size(), std::size_t{3});
    REQUIRE_EQ(sonorities[1].pitches[0], vll::Pitch{60});
    REQUIRE_EQ(sonorities[1].pitches[1], vll::Pitch{65});
    REQUIRE_EQ(sonorities[1].pitches[2], vll::Pitch{69});
}

TEST_CASE("sustain separates physically held notes from sounding notes") {
    vll::performance::SonorityDetector detector({80'000, 2, 16});
    detector.ingest(note(vll::NoteEventType::NoteOn, 60, 0));
    detector.ingest(note(vll::NoteEventType::NoteOn, 64, 10'000));
    detector.ingest({vll::NoteEventType::SustainOn, {}, 0.0F, 20'000, 1});
    detector.ingest(note(vll::NoteEventType::NoteOff, 60, 30'000));
    detector.ingest(note(vll::NoteEventType::NoteOff, 64, 40'000));
    detector.advanceTime(80'000);

    REQUIRE(detector.heldNotes().empty());
    REQUIRE_EQ(detector.soundingNotes().size(), std::size_t{2});
    REQUIRE_EQ(detector.completedSonorities().size(), std::size_t{1});

    detector.ingest({vll::NoteEventType::SustainOff, {}, 0.0F, 90'000, 1});
    REQUIRE(detector.soundingNotes().empty());
}

TEST_CASE("a repeated note starts a new sonority without duplicating pitch state") {
    vll::performance::SonorityDetector detector({50'000, 2, 16});
    detector.ingest(note(vll::NoteEventType::NoteOn, 60, 0));
    detector.ingest(note(vll::NoteEventType::NoteOn, 64, 5'000));
    detector.ingest(note(vll::NoteEventType::NoteOn, 67, 10'000));
    detector.advanceTime(50'000);

    detector.ingest(note(vll::NoteEventType::NoteOff, 64, 70'000));
    detector.ingest(note(vll::NoteEventType::NoteOn, 64, 75'000));
    detector.advanceTime(125'000);

    const auto sonorities = detector.completedSonorities();
    REQUIRE_EQ(sonorities.size(), std::size_t{2});
    REQUIRE_EQ(sonorities[1].pitches.size(), std::size_t{3});
    REQUIRE_EQ(detector.heldNotes().size(), std::size_t{3});
}

TEST_CASE("sonority history is bounded") {
    vll::performance::SonorityDetector detector({10, 2, 2});
    for (int chord = 0; chord < 3; ++chord) {
        const auto start = static_cast<vll::TimestampMicros>(chord * 100);
        detector.ingest(note(vll::NoteEventType::NoteOn, 60 + chord, start));
        detector.ingest(note(vll::NoteEventType::NoteOn, 64 + chord, start + 1));
        detector.advanceTime(start + 10);
        detector.ingest(note(vll::NoteEventType::NoteOff, 60 + chord, start + 20));
        detector.ingest(note(vll::NoteEventType::NoteOff, 64 + chord, start + 21));
    }
    const auto sonorities = detector.completedSonorities();
    REQUIRE_EQ(sonorities.size(), std::size_t{2});
    REQUIRE_EQ(sonorities.front().pitches.front(), vll::Pitch{61});
}
