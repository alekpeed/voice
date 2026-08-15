#include "TestFramework.h"
#include "vll/notation/SvgNotationRenderer.h"

#include <algorithm>
#include <cstddef>
#include <string>

namespace {

vll::notation::NotationDocument iiVINotation() {
    return {
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
}

vll::notation::NotationDocument repeatedEvents(const int count) {
    vll::notation::NotationDocument document;
    for (int index = 0; index < count; ++index) {
        document.events.push_back({
            static_cast<vll::TimestampMicros>(index) * 500'000,
            {{{48 + index % 5}, 1, std::nullopt, false},
             {{60 + index % 5}, 2, std::nullopt, false}},
            index % 2 == 0 ? "C" : "G7",
            1.0,
        });
    }
    return document;
}

} // namespace

TEST_CASE("three-event ii-V-I uses a compact grand-staff system") {
    const vll::notation::SvgNotationRenderer renderer;
    const auto rendered = renderer.render(iiVINotation(), {});
    REQUIRE_EQ(rendered.systemCount, std::size_t{1});
    REQUIRE(rendered.width < 500.0F);
    REQUIRE(rendered.width < 960.0F);
    REQUIRE(rendered.height > 200.0F);
    REQUIRE(rendered.scalableVectorData.find("Grand-staff notation") != std::string::npos);
    REQUIRE(rendered.scalableVectorData.find("Dm9") != std::string::npos);
    REQUIRE(rendered.scalableVectorData.find("G13") != std::string::npos);
    REQUIRE(rendered.scalableVectorData.find("Cmaj9") != std::string::npos);
}

TEST_CASE("ii-V-I layout splits bass and upper voices across the grand staff") {
    const auto layout = vll::notation::SvgNotationRenderer{}.layout(iiVINotation(), {});
    REQUIRE_EQ(layout.systems.size(), std::size_t{1});
    const auto& notes = layout.systems.front().notes;
    REQUIRE_EQ(notes.size(), std::size_t{12});
    for (const auto& note : notes) {
        if (note.voiceId <= 2) REQUIRE_EQ(note.staff, vll::notation::Staff::Bass);
        else REQUIRE_EQ(note.staff, vll::notation::Staff::Treble);
    }
    const auto middleC = std::ranges::find(notes, vll::Pitch{60}, &vll::notation::NoteGlyph::pitch);
    REQUIRE(middleC != notes.end());
    REQUIRE_EQ(middleC->staff, vll::notation::Staff::Treble);
    REQUIRE(!middleC->ledgerLines.empty());
}

TEST_CASE("accidental state suppresses repeats and restores naturals") {
    vll::notation::NotationDocument document{{
        {0, {{{61}, 2, std::nullopt, false}}, "", 1.0},
        {1, {{{61}, 2, std::nullopt, false}}, "", 1.0},
        {2, {{{60}, 2, std::nullopt, false}}, "", 1.0},
    }, std::nullopt};
    const auto layout = vll::notation::SvgNotationRenderer{}.layout(document, {});
    const auto& notes = layout.systems.front().notes;
    REQUIRE_EQ(notes[0].accidental, std::string{"sharp"});
    REQUIRE(notes[1].accidental.empty());
    REQUIRE_EQ(notes[2].accidental, std::string{"natural"});
}

TEST_CASE("flat spelling preference produces flat accidentals") {
    vll::notation::NotationDocument document{{
        {0, {{{61}, 2, std::nullopt, false}, {{63}, 3, std::nullopt, false}}, "Db", 1.0},
    }, std::nullopt};
    vll::notation::EngravingOptions options;
    options.accidentalPreference = vll::notation::AccidentalPreference::Flats;
    const auto layout = vll::notation::SvgNotationRenderer{}.layout(document, options);
    REQUIRE_EQ(layout.systems[0].notes[0].pitchName, std::string{"Db4"});
    REQUIRE_EQ(layout.systems[0].notes[0].accidental, std::string{"flat"});
    REQUIRE_EQ(layout.systems[0].notes[1].pitchName, std::string{"Eb4"});
}

TEST_CASE("seconds and adjacent accidentals receive collision offsets") {
    vll::notation::NotationDocument document{{
        {0, {{{61}, 3, std::nullopt, false}, {{63}, 4, std::nullopt, false}}, "D7alt", 1.0},
    }, std::nullopt};
    const auto notes = vll::notation::SvgNotationRenderer{}.layout(document, {}).systems[0].notes;
    REQUIRE_EQ(notes.size(), std::size_t{2});
    REQUIRE(notes[0].noteheadOffset != notes[1].noteheadOffset);
    REQUIRE(notes[0].accidentalColumn != notes[1].accidentalColumn);
}

TEST_CASE("ledger lines cover notes outside both staves") {
    vll::notation::NotationDocument document{{
        {0, {{{24}, 1, std::nullopt, false}, {{84}, 4, std::nullopt, false}}, "C", 1.0},
    }, std::nullopt};
    const auto notes = vll::notation::SvgNotationRenderer{}.layout(document, {}).systems[0].notes;
    REQUIRE(notes[0].ledgerLines.size() >= std::size_t{2});
    REQUIRE(notes[1].ledgerLines.size() >= std::size_t{2});
}

TEST_CASE("maximum width wraps long notation without stretching the final system") {
    vll::notation::EngravingOptions options;
    options.maximumSystemWidth = 420.0F;
    const auto rendered = vll::notation::SvgNotationRenderer{}.render(repeatedEvents(10), options);
    REQUIRE_EQ(rendered.systemCount, std::size_t{5});
    REQUIRE(rendered.width <= 420.0F);
    REQUIRE(rendered.contentWidth <= 420.0F);
}

TEST_CASE("notation scale changes geometry proportionally") {
    auto smallOptions = vll::notation::EngravingOptions{};
    smallOptions.scale = 0.75F;
    auto largeOptions = smallOptions;
    largeOptions.scale = 1.5F;
    const auto small = vll::notation::SvgNotationRenderer{}.render(iiVINotation(), smallOptions);
    const auto large = vll::notation::SvgNotationRenderer{}.render(iiVINotation(), largeOptions);
    REQUIRE(std::abs(large.width / small.width - 2.0F) < 0.01F);
    REQUIRE(std::abs(large.height / small.height - 2.0F) < 0.01F);
}

TEST_CASE("playback cursor interpolates between chord events") {
    const auto layout = vll::notation::SvgNotationRenderer{}.layout(iiVINotation(), {});
    const auto& system = layout.systems.front();
    REQUIRE(system.playbackCursorX.has_value());
    const float midpoint = (system.events[0].x + system.events[1].x) / 2.0F;
    REQUIRE(std::abs(*system.playbackCursorX - midpoint) < 0.01F);
}

TEST_CASE("engraving toggles chord symbols analysis marks fingering and highlights") {
    auto options = vll::notation::EngravingOptions{};
    options.showChordSymbols = false;
    options.showAnalysisMarks = false;
    options.showFingering = false;
    options.showVoiceHighlights = false;
    const auto svg = vll::notation::SvgNotationRenderer{}.render(iiVINotation(), options).scalableVectorData;
    REQUIRE(svg.find("Dm9") == std::string::npos);
    REQUIRE(svg.find(">V3</text>") == std::string::npos);
    REQUIRE(svg.find("voice-highlight") == std::string::npos);
    REQUIRE(svg.find(">5</text>") == std::string::npos);
}

TEST_CASE("SVG contains clefs accidentals ledger lines highlights fingering and cursor") {
    auto document = iiVINotation();
    document.events[0].notes[3].pitch = vll::Pitch{66};
    const auto svg = vll::notation::SvgNotationRenderer{}.render(document, {}).scalableVectorData;
    REQUIRE(svg.starts_with("<svg"));
    REQUIRE(svg.find("data-clef=\"treble\"") != std::string::npos);
    REQUIRE(svg.find("data-clef=\"bass\"") != std::string::npos);
    REQUIRE(svg.find("data-accidental=\"sharp\"") != std::string::npos);
    REQUIRE(svg.find("data-role=\"ledger-line\"") != std::string::npos);
    REQUIRE(svg.find("data-role=\"voice-highlight\"") != std::string::npos);
    REQUIRE(svg.find("data-role=\"playback-cursor\"") != std::string::npos);
    REQUIRE(svg.find(">5</text>") != std::string::npos);
}

TEST_CASE("empty notation document returns no rendered surface") {
    const auto rendered = vll::notation::SvgNotationRenderer{}.render({}, {});
    REQUIRE(rendered.scalableVectorData.empty());
    REQUIRE_EQ(rendered.width, 0.0F);
    REQUIRE_EQ(rendered.systemCount, std::size_t{0});
}
