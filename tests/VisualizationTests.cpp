#include "TestFramework.h"
#include "vll/analysis/VoiceAssigner.h"
#include "vll/visualization/VisualizationModel.h"
#include "vll/visualization/VoiceGraphSvgRenderer.h"
#include "vll/visualization/VoicePathBuilder.h"

#include <cstddef>
#include <string>
#include <vector>

namespace {

std::vector<vll::Sonority> iiVI() {
    return {
        {{{50}, {53}, {60}, {64}}, 0, 80'000},
        {{{43}, {53}, {59}, {62}}, 1'000'000, 1'080'000},
        {{{48}, {52}, {59}, {62}}, 2'000'000, 2'080'000},
    };
}

vll::ExerciseConstraint fourVoices() {
    vll::ExerciseConstraint constraints;
    constraints.voiceCount = 4;
    constraints.maximumLeap = 12;
    return constraints;
}

vll::visualization::VoicePathBuildResult buildIiVI() {
    static const vll::analysis::VoiceAssigner assigner;
    return vll::visualization::VoicePathBuilder(assigner).build(iiVI(), fourVoices());
}

} // namespace

TEST_CASE("ii-V-I assignments become persistent labeled voice paths") {
    const auto result = buildIiVI();
    REQUIRE(result.complete);
    REQUIRE_EQ(result.assignments.size(), std::size_t{2});
    REQUIRE_EQ(result.paths.size(), std::size_t{4});
    for (const auto& path : result.paths) REQUIRE_EQ(path.points.size(), std::size_t{3});

    REQUIRE_EQ(result.paths[0].voice.label, std::string{"Bass"});
    REQUIRE_EQ(result.paths[3].voice.label, std::string{"Soprano"});
    REQUIRE_EQ(result.paths[0].points[0].pitch, vll::Pitch{50});
    REQUIRE_EQ(result.paths[0].points[1].pitch, vll::Pitch{43});
    REQUIRE_EQ(result.paths[0].points[2].pitch, vll::Pitch{48});
    REQUIRE_EQ(result.paths[2].points[0].pitch, vll::Pitch{60});
    REQUIRE_EQ(result.paths[2].points[1].pitch, vll::Pitch{59});
    REQUIRE_EQ(result.paths[2].points[2].pitch, vll::Pitch{59});
}

TEST_CASE("visualization frame tracks cursor keyboard and voice isolation") {
    const auto built = buildIiVI();
    vll::visualization::VisualizationModel model(built.paths);
    model.setTimeline({{0, "ii"}, {1'000'000, "V"}, {2'000'000, "I"}});
    model.setCursor(1'000'000);

    auto frame = model.frame();
    REQUIRE_EQ(frame.highlightedPitches.size(), std::size_t{4});
    REQUIRE_EQ(frame.keyboardKeys.size(), std::size_t{4});
    REQUIRE_EQ(frame.timeline.size(), std::size_t{3});
    REQUIRE_EQ(frame.cursor, vll::TimestampMicros{1'000'000});

    model.isolateVoice(vll::VoiceId{3});
    frame = model.frame();
    REQUIRE_EQ(frame.isolatedVoice, std::optional<vll::VoiceId>{3});
    REQUIRE_EQ(frame.highlightedPitches.size(), std::size_t{1});
    REQUIRE_EQ(frame.highlightedPitches.front(), vll::Pitch{59});
    REQUIRE_EQ(frame.keyboardKeys.front().voiceId, vll::VoiceId{3});

    model.isolateVoice(vll::VoiceId{99});
    REQUIRE(!model.frame().isolatedVoice.has_value());
}

TEST_CASE("viewport clamps to content and can reset") {
    const auto built = buildIiVI();
    vll::visualization::VisualizationModel model(built.paths);
    model.setViewport(500'000, 1'500'000);
    auto frame = model.frame();
    REQUIRE_EQ(frame.visibleFrom, vll::TimestampMicros{500'000});
    REQUIRE_EQ(frame.visibleTo, vll::TimestampMicros{1'500'000});

    model.setCursor(9'000'000);
    REQUIRE_EQ(model.frame().cursor, vll::TimestampMicros{2'000'000});
    model.resetViewport();
    frame = model.frame();
    REQUIRE_EQ(frame.visibleFrom, vll::TimestampMicros{0});
    REQUIRE_EQ(frame.visibleTo, vll::TimestampMicros{2'000'000});
}

TEST_CASE("slow playback advances the cursor at the selected rate") {
    const auto built = buildIiVI();
    vll::visualization::VisualizationModel model(built.paths);
    model.setPlaybackRate(0.5);
    model.setPlaying(true);
    model.advancePlayback(1'000'000);
    auto frame = model.frame();
    REQUIRE_EQ(frame.cursor, vll::TimestampMicros{500'000});
    REQUIRE(frame.playing);
    REQUIRE_EQ(frame.playbackRate, 0.5);

    model.advancePlayback(4'000'000);
    frame = model.frame();
    REQUIRE_EQ(frame.cursor, vll::TimestampMicros{2'000'000});
    REQUIRE(!frame.playing);
}

TEST_CASE("isolated slow-playback plan contains only the selected voice") {
    const auto built = buildIiVI();
    vll::visualization::VisualizationModel model(built.paths);
    model.setPlaybackRate(0.5);
    model.isolateVoice(vll::VoiceId{3});
    const auto events = model.playbackEvents(250'000);
    REQUIRE_EQ(events.size(), std::size_t{4});
    for (const auto& event : events) REQUIRE_EQ(event.voiceId, vll::VoiceId{3});
    REQUIRE_EQ(events[0].type, vll::NoteEventType::NoteOn);
    REQUIRE_EQ(events[0].pitch, vll::Pitch{60});
    REQUIRE_EQ(events[1].timestamp, vll::TimestampMicros{2'000'000});
    REQUIRE_EQ(events.back().type, vll::NoteEventType::NoteOff);
    REQUIRE_EQ(events.back().timestamp, vll::TimestampMicros{4'250'000});
}

TEST_CASE("voice graph SVG labels paths movement events keyboard and cursor") {
    const auto built = buildIiVI();
    vll::visualization::VisualizationModel model(built.paths);
    model.setTimeline({{0, "ii"}, {1'000'000, "V"}, {2'000'000, "I"}});
    model.setCursor(1'000'000);
    model.isolateVoice(vll::VoiceId{3});

    const auto svg = vll::visualization::VoiceGraphSvgRenderer{}.render(model.frame());
    REQUIRE(svg.starts_with("<svg"));
    REQUIRE(svg.find("Voice-leading graph") != std::string::npos);
    REQUIRE(svg.find("V3 Alto") != std::string::npos);
    REQUIRE(svg.find("-1 semitone") != std::string::npos);
    REQUIRE(svg.find(">ii</text>") != std::string::npos);
    REQUIRE(svg.find(">V</text>") != std::string::npos);
    REQUIRE(svg.find(">I</text>") != std::string::npos);
    REQUIRE(svg.find("data-role=\"playback-cursor\"") != std::string::npos);
    REQUIRE(svg.find("opacity=\"0.18\"") != std::string::npos);
    REQUIRE(svg.find(">V3</text>") != std::string::npos);
}

TEST_CASE("voice graph SVG escapes timeline text") {
    const auto built = buildIiVI();
    vll::visualization::VisualizationModel model(built.paths);
    model.setTimeline({{0, "ii & V < I"}});
    const auto svg = vll::visualization::VoiceGraphSvgRenderer{}.render(model.frame());
    REQUIRE(svg.find("ii &amp; V &lt; I") != std::string::npos);
}

TEST_CASE("path building fails explicitly when a transition changes voice count") {
    static const vll::analysis::VoiceAssigner assigner;
    auto progression = iiVI();
    progression[1].pitches.pop_back();
    const auto result = vll::visualization::VoicePathBuilder(assigner).build(
        progression, fourVoices());
    REQUIRE(!result.complete);
    REQUIRE_EQ(result.failedTransition, std::size_t{0});
}

TEST_CASE("path building rejects a nonmonotonic event timeline") {
    static const vll::analysis::VoiceAssigner assigner;
    auto progression = iiVI();
    progression[1].startedAt = 0;
    const auto result = vll::visualization::VoicePathBuilder(assigner).build(
        progression, fourVoices());
    REQUIRE(!result.complete);
    REQUIRE_EQ(result.failedTransition, std::size_t{0});
}
