#include "vll/notation/SvgNotationRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace vll::notation {
namespace {

constexpr std::array<std::string_view, 6> kVoiceColors{
    "#1677a8", "#4e7d2b", "#a35f00", "#6b4fa3", "#a22c74", "#177f86"};

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

std::string accidentalEntity(const std::string_view accidental) {
    if (accidental == "sharp") return "&#x266F;";
    if (accidental == "flat") return "&#x266D;";
    if (accidental == "natural") return "&#x266E;";
    return {};
}

std::string_view voiceColor(const VoiceId voiceId) {
    if (voiceId == 0) return "#1b1b1b";
    return kVoiceColors[static_cast<std::size_t>(voiceId - 1) % kVoiceColors.size()];
}

} // namespace

RenderedNotation SvgNotationRenderer::render(const NotationDocument& document,
                                             const EngravingOptions& options) const {
    const auto engraving = layout(document, options);
    if (engraving.systems.empty()) return {};

    const float scale = engraving.scale;
    const float noteRadiusX = 7.2F * scale;
    const float noteRadiusY = 5.0F * scale;
    std::ostringstream svg;
    svg << std::fixed << std::setprecision(2);
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" role=\"img\" "
        << "aria-label=\"Grand-staff notation\" viewBox=\"0 0 "
        << engraving.width << ' ' << engraving.height << "\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"#f7f3e8\"/>\n";
    svg << "<style>text{font-family:Georgia,serif;fill:#191919}"
           ".music{font-family:Georgia,serif}"
           ".chord{font-family:system-ui,sans-serif;font-weight:650}"
           ".small{font-family:system-ui,sans-serif;font-size:9px;paint-order:stroke;"
           "stroke:#f7f3e8;stroke-width:3px;stroke-linejoin:round}"
           ".voice{font-weight:700}</style>\n";

    for (const auto& system : engraving.systems) {
        const float trebleTop = system.top + 60.0F * scale;
        const float trebleBottom = system.top + 100.0F * scale;
        const float bassTop = system.top + 150.0F * scale;
        const float bassBottom = system.top + 190.0F * scale;
        const float staffLeft = 34.0F * scale;
        const float staffRight = system.width - 14.0F * scale;

        svg << "<g data-system=\"" << (&system - engraving.systems.data()) << "\">\n";
        for (int line = 0; line < 5; ++line) {
            const float trebleY = trebleTop + static_cast<float>(line) * 10.0F * scale;
            const float bassY = bassTop + static_cast<float>(line) * 10.0F * scale;
            svg << "<line x1=\"" << staffLeft << "\" y1=\"" << trebleY << "\" x2=\""
                << staffRight << "\" y2=\"" << trebleY
                << "\" stroke=\"#292929\" stroke-width=\"" << 1.05F * scale << "\"/>\n";
            svg << "<line x1=\"" << staffLeft << "\" y1=\"" << bassY << "\" x2=\""
                << staffRight << "\" y2=\"" << bassY
                << "\" stroke=\"#292929\" stroke-width=\"" << 1.05F * scale << "\"/>\n";
        }
        svg << "<path d=\"M " << 30.0F * scale << ' ' << trebleTop
            << " C " << 14.0F * scale << ' ' << trebleTop << ", " << 25.0F * scale << ' '
            << (trebleBottom + bassTop) / 2.0F << ", " << 16.0F * scale << ' '
            << (trebleBottom + bassTop) / 2.0F
            << " C " << 25.0F * scale << ' ' << (trebleBottom + bassTop) / 2.0F << ", "
            << 14.0F * scale << ' ' << bassBottom << ", " << 30.0F * scale << ' ' << bassBottom
            << "\" fill=\"none\" stroke=\"#292929\" stroke-width=\"" << 2.0F * scale << "\"/>\n";
        svg << "<line x1=\"" << staffLeft << "\" y1=\"" << trebleTop << "\" x2=\""
            << staffLeft << "\" y2=\"" << bassBottom << "\" stroke=\"#292929\" stroke-width=\""
            << 1.7F * scale << "\"/>\n";
        svg << "<g data-clef=\"treble\" transform=\"translate(" << 75.0F * scale << ' '
            << trebleTop + 21.0F * scale << ") scale(" << scale << ")\">"
            << "<path d=\"M 4 -33 C -8 -22 -11 -7 0 7 C 12 22 14 37 8 48 "
               "C 4 55 -4 54 -8 48 C -11 42 -8 36 -2 36 C 4 36 6 41 4 45 "
               "M 4 -33 C 15 -22 15 -11 7 -2 C 1 5 -10 8 -17 1 "
               "C -24 -7 -20 -18 -10 -22 C 0 -26 9 -20 9 -12 "
               "C 9 -4 1 3 -8 2 C -15 1 -18 -5 -15 -11\" fill=\"none\" "
            << "stroke=\"#191919\" stroke-width=\"2.7\" stroke-linecap=\"round\" "
               "stroke-linejoin=\"round\"/></g>\n";
        svg << "<g data-clef=\"bass\" transform=\"translate(" << 67.0F * scale << ' '
            << bassTop + 18.0F * scale << ") scale(" << scale << ")\">"
            << "<path d=\"M -7 -12 C 9 -16 17 -5 11 9 C 7 19 -2 25 -13 25 "
               "C -4 20 1 13 1 6 C 1 -2 -3 -7 -8 -5 C -13 -3 -13 3 -9 5\" "
               "fill=\"none\" stroke=\"#191919\" stroke-width=\"2.7\" "
               "stroke-linecap=\"round\"/>"
            << "<circle cx=\"19\" cy=\"-3\" r=\"2.2\" fill=\"#191919\"/>"
            << "<circle cx=\"19\" cy=\"8\" r=\"2.2\" fill=\"#191919\"/></g>\n";
        svg << "<text x=\"" << 105.0F * scale << "\" y=\"" << trebleTop + 19.0F * scale
            << "\" font-size=\"" << 18.0F * scale << "px\">4</text>\n";
        svg << "<text x=\"" << 105.0F * scale << "\" y=\"" << trebleTop + 39.0F * scale
            << "\" font-size=\"" << 18.0F * scale << "px\">4</text>\n";
        svg << "<text x=\"" << 105.0F * scale << "\" y=\"" << bassTop + 19.0F * scale
            << "\" font-size=\"" << 18.0F * scale << "px\">4</text>\n";
        svg << "<text x=\"" << 105.0F * scale << "\" y=\"" << bassTop + 39.0F * scale
            << "\" font-size=\"" << 18.0F * scale << "px\">4</text>\n";

        for (const auto& event : system.events) {
            if (!event.chordSymbol.empty()) {
                svg << "<text class=\"chord\" text-anchor=\"middle\" x=\"" << event.x
                    << "\" y=\"" << system.top + 25.0F * scale << "\" font-size=\""
                    << 15.0F * scale << "px\">" << escapeXml(event.chordSymbol) << "</text>\n";
            }
            if ((event.eventIndex + 1) % 4 == 0 &&
                event.eventIndex + 1 < document.events.size()) {
                const float barX = event.x + 43.0F * scale;
                svg << "<line x1=\"" << barX << "\" y1=\"" << trebleTop << "\" x2=\""
                    << barX << "\" y2=\"" << bassBottom << "\" stroke=\"#292929\" stroke-width=\""
                    << 1.4F * scale << "\"/>\n";
            }
        }

        std::map<std::pair<std::size_t, Staff>, std::vector<const NoteGlyph*>> chordNotes;
        for (const auto& note : system.notes) chordNotes[{note.eventIndex, note.staff}].push_back(&note);
        for (const auto& [key, notes] : chordNotes) {
            if (notes.empty() || notes.front()->durationBeats >= 4.0) continue;
            float averageY = 0.0F;
            float minimumY = notes.front()->y;
            float maximumY = notes.front()->y;
            float minimumX = notes.front()->x + notes.front()->noteheadOffset;
            float maximumX = minimumX;
            for (const auto* note : notes) {
                averageY += note->y;
                minimumY = std::min(minimumY, note->y);
                maximumY = std::max(maximumY, note->y);
                minimumX = std::min(minimumX, note->x + note->noteheadOffset);
                maximumX = std::max(maximumX, note->x + note->noteheadOffset);
            }
            averageY /= static_cast<float>(notes.size());
            const float staffCenter = key.second == Staff::Treble
                                          ? (trebleTop + trebleBottom) / 2.0F
                                          : (bassTop + bassBottom) / 2.0F;
            const bool stemUp = averageY >= staffCenter;
            const float stemX = stemUp ? maximumX + noteRadiusX : minimumX - noteRadiusX;
            const float stemStartY = stemUp ? maximumY : minimumY;
            const float stemEndY = stemUp ? minimumY - 30.0F * scale : maximumY + 30.0F * scale;
            svg << "<line x1=\"" << stemX << "\" y1=\"" << stemStartY << "\" x2=\""
                << stemX << "\" y2=\"" << stemEndY << "\" stroke=\"#191919\" stroke-width=\""
                << 1.4F * scale << "\"/>\n";
        }

        for (const auto& note : system.notes) {
            const float noteX = note.x + note.noteheadOffset;
            for (const float ledgerY : note.ledgerLines) {
                svg << "<line data-role=\"ledger-line\" x1=\"" << noteX - 12.0F * scale
                    << "\" y1=\"" << ledgerY << "\" x2=\"" << noteX + 12.0F * scale
                    << "\" y2=\"" << ledgerY << "\" stroke=\"#191919\" stroke-width=\""
                    << 1.2F * scale << "\"/>\n";
            }
            if (!note.accidental.empty()) {
                const float accidentalX = noteX - 17.0F * scale -
                                          static_cast<float>(note.accidentalColumn) * 10.0F * scale;
                svg << "<text class=\"music\" data-accidental=\"" << note.accidental
                    << "\" text-anchor=\"middle\" x=\"" << accidentalX << "\" y=\""
                    << note.y + 6.0F * scale << "\" font-size=\"" << 19.0F * scale << "px\">"
                    << accidentalEntity(note.accidental) << "</text>\n";
            }
            if (note.highlighted) {
                svg << "<ellipse data-role=\"voice-highlight\" cx=\"" << noteX << "\" cy=\""
                    << note.y << "\" rx=\"" << noteRadiusX + 4.0F * scale << "\" ry=\""
                    << noteRadiusY + 4.0F * scale << "\" fill=\"none\" stroke=\""
                    << voiceColor(note.voiceId) << "\" stroke-width=\"" << 2.2F * scale << "\"/>\n";
            }
            const bool openNote = note.durationBeats >= 2.0;
            svg << "<ellipse data-pitch=\"" << escapeXml(note.pitchName) << "\" data-voice=\""
                << note.voiceId << "\" cx=\"" << noteX << "\" cy=\"" << note.y << "\" rx=\""
                << noteRadiusX << "\" ry=\"" << noteRadiusY << "\" transform=\"rotate(-18 "
                << noteX << ' ' << note.y << ")\" fill=\"" << (openNote ? "#f7f3e8" : "#191919")
                << "\" stroke=\"#191919\" stroke-width=\"" << 1.4F * scale << "\">"
                << "<title>" << escapeXml(note.pitchName) << ", voice " << note.voiceId
                << "</title></ellipse>\n";
            if (options.showAnalysisMarks && note.voiceId > 0) {
                svg << "<text class=\"small voice\" x=\"" << noteX + 13.0F * scale
                    << "\" y=\"" << note.y + 3.0F * scale << "\" style=\"fill:"
                    << voiceColor(note.voiceId) << "\">V" << note.voiceId << "</text>\n";
            }
            if (note.fingering) {
                svg << "<text class=\"small\" text-anchor=\"end\" x=\""
                    << noteX - 11.0F * scale << "\" y=\"" << note.y + 3.0F * scale
                    << "\">" << *note.fingering << "</text>\n";
            }
        }

        if (system.playbackCursorX) {
            svg << "<line data-role=\"playback-cursor\" x1=\"" << *system.playbackCursorX
                << "\" y1=\"" << system.top + 34.0F * scale << "\" x2=\""
                << *system.playbackCursorX << "\" y2=\"" << bassBottom + 18.0F * scale
                << "\" stroke=\"#b3261e\" stroke-width=\"" << 2.0F * scale
                << "\" stroke-opacity=\"0.78\"/>\n";
        }
        svg << "</g>\n";
    }
    svg << "</svg>\n";
    return {svg.str(), engraving.width, engraving.height, engraving.systems.size(),
            engraving.contentWidth};
}

NotationLayout SvgNotationRenderer::layout(const NotationDocument& document,
                                            const EngravingOptions& options) const {
    return NotationLayouter{}.layout(document, options);
}

} // namespace vll::notation
