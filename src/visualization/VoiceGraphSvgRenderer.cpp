#include "vll/visualization/VoiceGraphSvgRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace vll::visualization {
namespace {

constexpr std::array<std::string_view, 6> kVoiceColors{
    "#66d9ef", "#a6e22e", "#f4bf75", "#ae81ff", "#fd5ff0", "#7bdff2"};

std::string escapeXml(const std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (const char character : text) {
        if (character == '&') escaped += "&amp;";
        else if (character == '<') escaped += "&lt;";
        else if (character == '>') escaped += "&gt;";
        else if (character == '\"') escaped += "&quot;";
        else if (character == '\'') escaped += "&apos;";
        else escaped += character;
    }
    return escaped;
}

std::vector<VoicePoint> pointsForViewport(const VoicePath& path,
                                          const TimestampMicros from,
                                          const TimestampMicros to) {
    if (path.points.empty()) return {};
    const auto first = std::ranges::lower_bound(path.points, from, {}, &VoicePoint::timestamp);
    const auto after = std::ranges::upper_bound(path.points, to, {}, &VoicePoint::timestamp);
    auto begin = first;
    auto end = after;
    if (begin != path.points.begin()) --begin;
    if (end != path.points.end()) ++end;
    return {begin, end};
}

std::string movementLabel(const int semitones) {
    if (semitones == 0) return "0 stationary";
    std::ostringstream label;
    if (semitones > 0) label << '+';
    label << semitones << ' ';
    const int distance = std::abs(semitones);
    if (distance == 1) label << "semitone";
    else if (distance == 2) label << "step";
    else label << "leap";
    return label.str();
}

bool isBlackKey(const int midiNote) {
    const int pitchClass = ((midiNote % 12) + 12) % 12;
    return pitchClass == 1 || pitchClass == 3 || pitchClass == 6 ||
           pitchClass == 8 || pitchClass == 10;
}

std::string pitchName(const int midiNote) {
    static constexpr std::array<std::string_view, 12> names{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const int pitchClass = ((midiNote % 12) + 12) % 12;
    return std::string{names[static_cast<std::size_t>(pitchClass)]} +
           std::to_string(midiNote / 12 - 1);
}

} // namespace

std::string VoiceGraphSvgRenderer::render(const Frame& frame,
                                          VoiceGraphRenderOptions options) const {
    options.width = std::max(options.width, 640);
    options.height = std::max(options.height, 420);
    const double left = 72.0;
    const double right = static_cast<double>(options.width) - 28.0;
    const double top = 36.0;
    const double graphBottom = static_cast<double>(options.height) - 170.0;
    const double timelineY = graphBottom + 42.0;
    const double keyboardY = graphBottom + 72.0;
    const double keyboardHeight = 64.0;
    const TimestampMicros visibleFrom = frame.visibleFrom;
    const TimestampMicros visibleTo = std::max(frame.visibleTo, visibleFrom + 1);

    int minimumPitch = std::numeric_limits<int>::max();
    int maximumPitch = std::numeric_limits<int>::min();
    for (const auto& path : frame.voicePaths) {
        for (const auto& point : pointsForViewport(path, visibleFrom, visibleTo)) {
            minimumPitch = std::min(minimumPitch, point.pitch.midiNote);
            maximumPitch = std::max(maximumPitch, point.pitch.midiNote);
        }
    }
    if (minimumPitch == std::numeric_limits<int>::max()) {
        minimumPitch = 60;
        maximumPitch = 72;
    }
    minimumPitch -= 1;
    maximumPitch += 1;
    const int pitchRange = std::max(1, maximumPitch - minimumPitch);

    const auto xForTime = [&](const TimestampMicros timestamp) {
        const double ratio = static_cast<double>(timestamp - visibleFrom) /
                             static_cast<double>(visibleTo - visibleFrom);
        return left + std::clamp(ratio, 0.0, 1.0) * (right - left);
    };
    const auto yForPitch = [&](const Pitch pitch) {
        const double ratio = static_cast<double>(pitch.midiNote - minimumPitch) /
                             static_cast<double>(pitchRange);
        return graphBottom - ratio * (graphBottom - top);
    };

    std::ostringstream svg;
    svg << std::fixed << std::setprecision(2);
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" role=\"img\" "
        << "aria-label=\"Voice-leading graph\" viewBox=\"0 0 "
        << options.width << ' ' << options.height << "\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"#10141c\"/>\n";
    svg << "<style>text{font-family:system-ui,sans-serif;fill:#d9e1ee}"
           ".small{font-size:11px}.label{font-size:13px;font-weight:600}</style>\n";

    const int gridStep = pitchRange > 24 ? 6 : pitchRange > 12 ? 3 : 2;
    for (int pitch = minimumPitch; pitch <= maximumPitch; pitch += gridStep) {
        const double y = yForPitch(Pitch{pitch});
        svg << "<line x1=\"" << left << "\" y1=\"" << y << "\" x2=\"" << right
            << "\" y2=\"" << y << "\" stroke=\"#263143\" stroke-width=\"1\"/>\n";
        svg << "<text class=\"small\" x=\"12\" y=\"" << y + 4.0 << "\">"
            << pitchName(pitch) << " · " << pitch << "</text>\n";
    }

    for (const auto& marker : frame.timeline) {
        if (marker.timestamp < visibleFrom || marker.timestamp > visibleTo) continue;
        const double x = xForTime(marker.timestamp);
        svg << "<line x1=\"" << x << "\" y1=\"" << top << "\" x2=\"" << x
            << "\" y2=\"" << graphBottom << "\" stroke=\"#637083\" "
            << "stroke-opacity=\"0.35\" stroke-dasharray=\"4 6\"/>\n";
        svg << "<text class=\"label\" text-anchor=\"middle\" x=\"" << x
            << "\" y=\"" << timelineY << "\">" << escapeXml(marker.label) << "</text>\n";
    }

    for (std::size_t pathIndex = 0; pathIndex < frame.voicePaths.size(); ++pathIndex) {
        const auto& path = frame.voicePaths[pathIndex];
        const auto points = pointsForViewport(path, visibleFrom, visibleTo);
        if (points.empty()) continue;
        const auto color = kVoiceColors[pathIndex % kVoiceColors.size()];
        const bool dimmed = frame.isolatedVoice && path.voice.id != *frame.isolatedVoice;
        const double opacity = dimmed ? 0.18 : 1.0;
        svg << "<g data-voice=\"" << path.voice.id << "\" aria-label=\"Voice "
            << path.voice.id << ' ' << escapeXml(path.voice.label) << "\" opacity=\""
            << opacity << "\">\n";
        for (std::size_t index = 1; index < points.size(); ++index) {
            const auto& from = points[index - 1];
            const auto& to = points[index];
            const double x1 = xForTime(from.timestamp);
            const double y1 = yForPitch(from.pitch);
            const double x2 = xForTime(to.timestamp);
            const double y2 = yForPitch(to.pitch);
            const int movement = from.pitch.distanceTo(to.pitch);
            svg << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2
                << "\" y2=\"" << y2 << "\" stroke=\"" << color
                << "\" stroke-width=\"4\" stroke-linecap=\"round\">"
                << "<title>Voice " << path.voice.id << ": " << movementLabel(movement)
                << "</title></line>\n";
            if (options.showMovementLabels && !dimmed) {
                svg << "<text class=\"small\" text-anchor=\"middle\" x=\""
                    << (x1 + x2) / 2.0 << "\" y=\"" << (y1 + y2) / 2.0 - 7.0
                    << "\">" << movementLabel(movement) << "</text>\n";
            }
        }
        for (const auto& point : points) {
            svg << "<circle cx=\"" << xForTime(point.timestamp) << "\" cy=\""
                << yForPitch(point.pitch) << "\" r=\"6\" fill=\"#10141c\" stroke=\""
                << color << "\" stroke-width=\"3\"><title>" << escapeXml(path.voice.label)
                << ", " << pitchName(point.pitch.midiNote) << "</title></circle>\n";
        }
        svg << "<text class=\"label\" x=\"" << left + 8.0 << "\" y=\""
            << yForPitch(points.front().pitch) - 10.0 << "\">V" << path.voice.id << ' '
            << escapeXml(path.voice.label) << "</text>\n</g>\n";
    }

    const double cursorX = xForTime(frame.cursor);
    svg << "<line data-role=\"playback-cursor\" x1=\"" << cursorX << "\" y1=\""
        << top << "\" x2=\"" << cursorX << "\" y2=\"" << graphBottom
        << "\" stroke=\"#ffffff\" stroke-width=\"2\"/>\n";

    if (!frame.keyboardKeys.empty()) {
        const auto [minimumKey, maximumKey] = std::ranges::minmax_element(
            frame.keyboardKeys, {}, &KeyboardKeyState::pitch);
        int firstKey = std::max(0, minimumKey->pitch.midiNote - 2);
        int lastKey = std::min(127, maximumKey->pitch.midiNote + 2);
        while (firstKey > 0 && isBlackKey(firstKey)) --firstKey;
        while (lastKey < 127 && isBlackKey(lastKey)) ++lastKey;
        int whiteKeyCount = 0;
        for (int midiNote = firstKey; midiNote <= lastKey; ++midiNote) {
            if (!isBlackKey(midiNote)) ++whiteKeyCount;
        }
        const double whiteKeyWidth = (right - left) / static_cast<double>(whiteKeyCount);
        int whiteIndex = 0;
        for (int midiNote = firstKey; midiNote <= lastKey; ++midiNote) {
            if (isBlackKey(midiNote)) continue;
            const auto active = std::ranges::find(frame.keyboardKeys, Pitch{midiNote},
                                                  &KeyboardKeyState::pitch);
            const bool isActive = active != frame.keyboardKeys.end();
            const auto color = isActive
                                   ? kVoiceColors[static_cast<std::size_t>(active->voiceId - 1) % kVoiceColors.size()]
                                   : std::string_view{"#d9e1ee"};
            const double x = left + static_cast<double>(whiteIndex) * whiteKeyWidth;
            svg << "<rect x=\"" << x << "\" y=\"" << keyboardY << "\" width=\""
                << whiteKeyWidth - 1.0 << "\" height=\"" << keyboardHeight << "\" rx=\"3\" fill=\""
                << color << "\" stroke=\"#465064\"/>\n";
            if (isActive) {
                svg << "<text class=\"small\" text-anchor=\"middle\" x=\""
                    << x + whiteKeyWidth / 2.0 << "\" y=\"" << keyboardY + 50.0
                    << "\" fill=\"#10141c\">V" << active->voiceId << "</text>\n";
            }
            ++whiteIndex;
        }
        whiteIndex = 0;
        for (int midiNote = firstKey; midiNote <= lastKey; ++midiNote) {
            if (!isBlackKey(midiNote)) {
                ++whiteIndex;
                continue;
            }
            const auto active = std::ranges::find(frame.keyboardKeys, Pitch{midiNote},
                                                  &KeyboardKeyState::pitch);
            const bool isActive = active != frame.keyboardKeys.end();
            const auto color = isActive
                                   ? kVoiceColors[static_cast<std::size_t>(active->voiceId - 1) % kVoiceColors.size()]
                                   : std::string_view{"#252c3a"};
            const double blackKeyWidth = whiteKeyWidth * 0.62;
            const double x = left + static_cast<double>(whiteIndex) * whiteKeyWidth -
                             blackKeyWidth / 2.0;
            svg << "<rect x=\"" << x << "\" y=\"" << keyboardY << "\" width=\""
                << blackKeyWidth << "\" height=\"" << keyboardHeight * 0.62
                << "\" rx=\"2\" fill=\"" << color << "\" stroke=\"#10141c\"/>\n";
            if (isActive) {
                svg << "<text class=\"small\" text-anchor=\"middle\" x=\""
                    << x + blackKeyWidth / 2.0 << "\" y=\"" << keyboardY + 27.0
                    << "\">V" << active->voiceId << "</text>\n";
            }
        }
    }
    svg << "<text class=\"small\" x=\"" << left << "\" y=\""
        << static_cast<double>(options.height) - 18.0 << "\">Playback "
        << frame.playbackRate << "x" << (frame.isolatedVoice ? " | isolated voice" : " | all voices")
        << "</text>\n</svg>\n";
    return svg.str();
}

} // namespace vll::visualization
