#include "vll/audio/SamplePiano.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ranges>
#include <sstream>
#include <unordered_map>

namespace vll::audio {
namespace {

using Opcodes = std::unordered_map<std::string, std::string>;

std::string trim(std::string value) {
    const auto first = std::ranges::find_if(value, [](const unsigned char character) {
        return !std::isspace(character);
    });
    const auto last = std::ranges::find_if(value | std::views::reverse,
                                           [](const unsigned char character) {
                                               return !std::isspace(character);
                                           }).base();
    if (first >= last) return {};
    return std::string(first, last);
}

void parseOpcodes(const std::string& text, Opcodes& destination) {
    std::size_t position = 0;
    while (position < text.size()) {
        while (position < text.size() && std::isspace(static_cast<unsigned char>(text[position]))) ++position;
        const std::size_t keyStart = position;
        while (position < text.size() && text[position] != '=' &&
               !std::isspace(static_cast<unsigned char>(text[position]))) ++position;
        if (position >= text.size() || text[position] != '=') {
            while (position < text.size() && !std::isspace(static_cast<unsigned char>(text[position]))) ++position;
            continue;
        }
        const std::string key = text.substr(keyStart, position - keyStart);
        ++position;
        std::string value;
        if (position < text.size() && (text[position] == '"' || text[position] == '\'')) {
            const char quote = text[position++];
            const std::size_t valueStart = position;
            while (position < text.size() && text[position] != quote) ++position;
            value = text.substr(valueStart, position - valueStart);
            if (position < text.size()) ++position;
        } else {
            const std::size_t valueStart = position;
            while (position < text.size() && !std::isspace(static_cast<unsigned char>(text[position]))) ++position;
            value = text.substr(valueStart, position - valueStart);
        }
        if (!key.empty()) destination.insert_or_assign(key, value);
    }
}

int integer(const Opcodes& opcodes, const std::string& key, const int fallback) {
    const auto found = opcodes.find(key);
    if (found == opcodes.end()) return fallback;
    try { return std::stoi(found->second); } catch (...) { return fallback; }
}

float decimal(const Opcodes& opcodes, const std::string& key, const float fallback) {
    const auto found = opcodes.find(key);
    if (found == opcodes.end()) return fallback;
    try { return std::stof(found->second); } catch (...) { return fallback; }
}

int noteNumber(const std::string& value, const int fallback) {
    try {
        std::size_t parsed = 0;
        const int numeric = std::stoi(value, &parsed);
        if (parsed == value.size()) return std::clamp(numeric, 0, 127);
    } catch (...) {}
    if (value.size() < 2) return fallback;
    const char letter = static_cast<char>(std::tolower(static_cast<unsigned char>(value[0])));
    int pitchClass = 0;
    switch (letter) {
        case 'c': pitchClass = 0; break;
        case 'd': pitchClass = 2; break;
        case 'e': pitchClass = 4; break;
        case 'f': pitchClass = 5; break;
        case 'g': pitchClass = 7; break;
        case 'a': pitchClass = 9; break;
        case 'b': pitchClass = 11; break;
        default: return fallback;
    }
    std::size_t position = 1;
    if (position < value.size() && (value[position] == '#' || value[position] == 'b')) {
        pitchClass += value[position] == '#' ? 1 : -1;
        ++position;
    }
    try {
        const int octave = std::stoi(value.substr(position));
        return std::clamp((octave + 1) * 12 + pitchClass, 0, 127);
    } catch (...) { return fallback; }
}

int noteOpcode(const Opcodes& opcodes, const std::string& key, const int fallback) {
    const auto found = opcodes.find(key);
    return found == opcodes.end() ? fallback : noteNumber(found->second, fallback);
}

void appendWarning(PianoLoadResult& result, std::string warning) {
    if (result.warnings.size() < 100) result.warnings.push_back(std::move(warning));
}

} // namespace

PianoLoadResult SfzPianoLoader::load(const std::filesystem::path& sfzPath) {
    PianoLoadResult result;
    std::ifstream input(sfzPath);
    if (!input) {
        result.error = "Unable to open SFZ file: " + sfzPath.string();
        return result;
    }

    PianoDefinition definition;
    definition.name = sfzPath.stem().string();
    Opcodes global;
    Opcodes group;
    Opcodes region;
    Opcodes control;
    enum class Section { None, Control, Global, Group, Region };
    Section section = Section::None;
    std::filesystem::path defaultPath;
    std::unordered_map<std::string, std::shared_ptr<const AudioSample>> sampleCache;

    auto finalizeRegion = [&] {
        if (section != Section::Region || region.empty()) return;
        const auto sampleOpcode = region.find("sample");
        if (sampleOpcode == region.end()) {
            appendWarning(result, "Ignored region without sample opcode");
            region.clear();
            return;
        }
        std::string sampleName = sampleOpcode->second;
        std::replace(sampleName.begin(), sampleName.end(), '\\', '/');
        const auto samplePath = (sfzPath.parent_path() / defaultPath / sampleName).lexically_normal();
        const std::string cacheKey = samplePath.string();
        std::shared_ptr<const AudioSample> sample;
        if (const auto cached = sampleCache.find(cacheKey); cached != sampleCache.end()) {
            sample = cached->second;
        } else {
            auto loaded = WavReader::load(samplePath);
            if (!loaded.sample) {
                appendWarning(result, loaded.error);
                region.clear();
                return;
            }
            sample = std::make_shared<AudioSample>(std::move(*loaded.sample));
            sampleCache.emplace(cacheKey, sample);
        }

        PianoRegion pianoRegion;
        pianoRegion.sample = std::move(sample);
        if (const auto key = region.find("key"); key != region.end()) {
            const int note = noteNumber(key->second, 60);
            pianoRegion.lowKey = note;
            pianoRegion.highKey = note;
            pianoRegion.rootKey = note;
        }
        pianoRegion.lowKey = noteOpcode(region, "lokey", pianoRegion.lowKey);
        pianoRegion.highKey = noteOpcode(region, "hikey", pianoRegion.highKey);
        pianoRegion.rootKey = noteOpcode(region, "pitch_keycenter", pianoRegion.rootKey);
        pianoRegion.lowVelocity = std::clamp(integer(region, "lovel", 1), 1, 127);
        pianoRegion.highVelocity = std::clamp(integer(region, "hivel", 127), 1, 127);
        pianoRegion.tuneCents = decimal(region, "tune", 0.0F);
        pianoRegion.gainDecibels = decimal(region, "volume", 0.0F);
        pianoRegion.pan = std::clamp(decimal(region, "pan", 0.0F) / 100.0F, -1.0F, 1.0F);
        pianoRegion.releaseSeconds = std::clamp(decimal(region, "ampeg_release", 0.6F), 0.005F, 30.0F);
        if (const auto trigger = region.find("trigger"); trigger != region.end() &&
            trigger->second == "release") {
            pianoRegion.trigger = SampleTrigger::Release;
        }
        definition.regions.push_back(std::move(pianoRegion));
        region.clear();
    };

    auto beginSection = [&](const std::string& header) {
        finalizeRegion();
        if (header == "control") {
            section = Section::Control;
        } else if (header == "global") {
            section = Section::Global;
            global.clear();
        } else if (header == "group") {
            section = Section::Group;
            group = global;
        } else if (header == "region") {
            section = Section::Region;
            region = group.empty() ? global : group;
        } else {
            section = Section::None;
        }
    };

    std::string line;
    while (std::getline(input, line)) {
        if (const auto comment = line.find("//"); comment != std::string::npos) line.erase(comment);
        std::size_t position = 0;
        while (position < line.size()) {
            const auto headerStart = line.find('<', position);
            if (headerStart == std::string::npos) {
                const auto opcodes = line.substr(position);
                switch (section) {
                    case Section::Control: parseOpcodes(opcodes, control); break;
                    case Section::Global: parseOpcodes(opcodes, global); break;
                    case Section::Group: parseOpcodes(opcodes, group); break;
                    case Section::Region: parseOpcodes(opcodes, region); break;
                    case Section::None: break;
                }
                break;
            }
            if (headerStart > position) {
                const auto opcodes = line.substr(position, headerStart - position);
                switch (section) {
                    case Section::Control: parseOpcodes(opcodes, control); break;
                    case Section::Global: parseOpcodes(opcodes, global); break;
                    case Section::Group: parseOpcodes(opcodes, group); break;
                    case Section::Region: parseOpcodes(opcodes, region); break;
                    case Section::None: break;
                }
            }
            const auto headerEnd = line.find('>', headerStart + 1);
            if (headerEnd == std::string::npos) break;
            beginSection(trim(line.substr(headerStart + 1, headerEnd - headerStart - 1)));
            position = headerEnd + 1;
        }
        if (const auto found = control.find("default_path"); found != control.end()) {
            defaultPath = found->second;
            std::string normalized = defaultPath.string();
            std::replace(normalized.begin(), normalized.end(), '\\', '/');
            defaultPath = normalized;
        }
    }
    finalizeRegion();

    if (definition.regions.empty()) {
        result.error = "SFZ contained no loadable WAV regions";
        return result;
    }
    result.definition = std::move(definition);
    return result;
}

} // namespace vll::audio
