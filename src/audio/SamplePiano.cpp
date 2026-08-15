#include "vll/audio/SamplePiano.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <utility>

namespace vll::audio {

SamplePiano::SamplePiano() : velocityCurve_(VelocityCurve::linear()) {}

void SamplePiano::prepare(const double sampleRate, const std::size_t maximumBlockSize) {
    sampleRate_ = std::clamp(sampleRate, 8000.0, 384000.0);
    maximumBlockSize_ = std::max<std::size_t>(1, maximumBlockSize);
}

bool SamplePiano::loadPreset(const std::string& presetId) {
    const auto found = presets_.find(presetId);
    if (found == presets_.end()) return false;
    auto result = SfzPianoLoader::load(found->second);
    return result.definition && loadDefinition(std::move(*result.definition));
}

void SamplePiano::noteOn(const Pitch pitch, const float velocity) noexcept {
    enqueue({CommandType::NoteOn, std::clamp(pitch.midiNote, 0, 127),
             std::clamp(velocity, 0.0F, 1.0F)});
}

void SamplePiano::noteOff(const Pitch pitch) noexcept {
    enqueue({CommandType::NoteOff, std::clamp(pitch.midiNote, 0, 127), 0.0F});
}

void SamplePiano::pedal(const float amount) noexcept {
    enqueue({CommandType::Pedal, 0, std::clamp(amount, 0.0F, 1.0F)});
}

bool SamplePiano::setParameter(const std::string& parameterId, const float value) {
    if (parameterId == "master_gain") {
        masterGain_.store(std::clamp(value, 0.0F, 2.0F), std::memory_order_relaxed);
        return true;
    }
    return false;
}

void SamplePiano::renderAudio(const StereoBuffer output) noexcept {
    const std::size_t frames = std::min(output.left.size(), output.right.size());
    std::fill_n(output.left.begin(), frames, 0.0F);
    std::fill_n(output.right.begin(), frames, 0.0F);

    if (panicRequested_.exchange(false, std::memory_order_acq_rel)) {
        commandRead_.store(commandWrite_.load(std::memory_order_acquire), std::memory_order_release);
        applyCommand({CommandType::AllNotesOff, 0, 0.0F});
    }
    Command command{};
    while (dequeue(command)) applyCommand(command);

    for (auto& voice : voices_) renderVoice(voice, {output.left.first(frames), output.right.first(frames)});
    std::erase_if(voices_, [](const Voice& voice) { return voice.region == nullptr; });
    activeVoiceCount_.store(voices_.size(), std::memory_order_relaxed);
}

void SamplePiano::allNotesOff() noexcept { enqueue({CommandType::AllNotesOff, 0, 0.0F}); }
int SamplePiano::latencySamples() const noexcept { return 0; }
bool SamplePiano::savePreset(const std::string&) { return false; }

bool SamplePiano::loadDefinition(PianoDefinition definition) {
    if (definition.regions.empty() || definition.maximumVoices == 0) return false;
    for (const auto& region : definition.regions) {
        if (!region.sample || !region.sample->valid() || region.lowKey > region.highKey ||
            region.lowVelocity > region.highVelocity) return false;
    }
    allNotesOff();
    voices_.clear();
    activeVoiceCount_.store(0, std::memory_order_relaxed);
    commandRead_.store(commandWrite_.load(std::memory_order_relaxed), std::memory_order_relaxed);
    definition_ = std::move(definition);
    voices_.reserve(definition_.maximumVoices);
    return true;
}

void SamplePiano::registerPreset(std::string id, std::filesystem::path sfzPath) {
    presets_.insert_or_assign(std::move(id), std::move(sfzPath));
}

void SamplePiano::setVelocityCurve(VelocityCurve curve) { velocityCurve_ = std::move(curve); }

std::size_t SamplePiano::activeVoiceCount() const noexcept {
    return activeVoiceCount_.load(std::memory_order_relaxed);
}

const std::string& SamplePiano::loadedPresetName() const noexcept { return definition_.name; }

bool SamplePiano::enqueue(const Command command) noexcept {
    const auto write = commandWrite_.load(std::memory_order_relaxed);
    const auto next = (write + 1) % commandCapacity_;
    if (next == commandRead_.load(std::memory_order_acquire)) {
        panicRequested_.store(true, std::memory_order_release);
        return false;
    }
    commands_[write] = command;
    commandWrite_.store(next, std::memory_order_release);
    return true;
}

bool SamplePiano::dequeue(Command& command) noexcept {
    const auto read = commandRead_.load(std::memory_order_relaxed);
    if (read == commandWrite_.load(std::memory_order_acquire)) return false;
    command = commands_[read];
    commandRead_.store((read + 1) % commandCapacity_, std::memory_order_release);
    return true;
}

void SamplePiano::applyCommand(const Command& command) noexcept {
    switch (command.type) {
        case CommandType::NoteOn:
            startRegions(command.note, command.value, SampleTrigger::Attack);
            break;
        case CommandType::NoteOff:
            beginRelease(command.note);
            break;
        case CommandType::Pedal: {
            const bool wasHolding = sustainAmount_ >= 0.5F;
            sustainAmount_ = command.value;
            if (wasHolding && sustainAmount_ < 0.5F) {
                for (auto& voice : voices_) {
                    if (voice.keyReleased && !voice.releasing) beginVoiceRelease(voice);
                }
            }
            break;
        }
        case CommandType::AllNotesOff:
            sustainAmount_ = 0.0F;
            for (auto& voice : voices_) beginVoiceRelease(voice);
            break;
    }
}

void SamplePiano::startRegions(const int note, const float velocity,
                               const SampleTrigger trigger) noexcept {
    const int midiVelocity = std::clamp(static_cast<int>(std::lround(velocity * 127.0F)), 1, 127);
    for (const auto& region : definition_.regions) {
        if (region.trigger != trigger || note < region.lowKey || note > region.highKey ||
            midiVelocity < region.lowVelocity || midiVelocity > region.highVelocity) continue;
        stealVoiceIfNeeded();
        if (voices_.size() >= definition_.maximumVoices) return;

        const float shapedVelocity = velocityCurve_.apply(velocity);
        const float gain = std::pow(10.0F, region.gainDecibels / 20.0F) * shapedVelocity *
                           masterGain_.load(std::memory_order_relaxed);
        const float pan = std::clamp(region.pan, -1.0F, 1.0F);
        const float angle = (pan + 1.0F) * std::numbers::pi_v<float> * 0.25F;
        Voice voice;
        voice.region = &region;
        voice.position = 0.0;
        voice.increment = std::pow(2.0, (static_cast<double>(note - region.rootKey) +
                                         static_cast<double>(region.tuneCents) / 100.0) / 12.0) *
                          static_cast<double>(region.sample->sampleRate) / sampleRate_;
        voice.leftGain = std::cos(angle) * gain;
        voice.rightGain = std::sin(angle) * gain;
        voice.envelope = 0.0F;
        voice.attackIncrement = 1.0F / static_cast<float>(std::max(1.0, sampleRate_ * 0.003));
        const double releaseSamples = std::max(1.0, sampleRate_ * region.releaseSeconds);
        voice.releaseMultiplier = static_cast<float>(std::pow(0.001, 1.0 / releaseSamples));
        voice.attackVelocity = velocity;
        voice.note = note;
        voice.releasing = false;
        voices_.push_back(voice);
    }
    activeVoiceCount_.store(voices_.size(), std::memory_order_relaxed);
}

void SamplePiano::beginRelease(const int note) noexcept {
    float attackVelocity = 0.5F;
    for (auto& voice : voices_) {
        if (voice.note != note || voice.region->trigger != SampleTrigger::Attack) continue;
        attackVelocity = std::max(attackVelocity, voice.attackVelocity);
        voice.keyReleased = true;
        if (sustainAmount_ < 0.5F) beginVoiceRelease(voice);
    }
    startRegions(note, attackVelocity, SampleTrigger::Release);
}

void SamplePiano::beginVoiceRelease(Voice& voice) noexcept {
    voice.releasing = true;
    voice.keyReleased = true;
}

void SamplePiano::renderVoice(Voice& voice, const StereoBuffer output) noexcept {
    if (voice.region == nullptr) return;
    const auto& sample = *voice.region->sample;
    for (std::size_t frame = 0; frame < output.left.size(); ++frame) {
        const auto position = static_cast<std::size_t>(voice.position);
        if (position + 1 >= sample.frameCount() ||
            (voice.releasing && voice.envelope < 0.00001F)) {
            voice.region = nullptr;
            return;
        }
        const float fraction = static_cast<float>(voice.position - static_cast<double>(position));
        const float left = sample.left[position] + fraction * (sample.left[position + 1] - sample.left[position]);
        const float right = sample.right[position] + fraction * (sample.right[position + 1] - sample.right[position]);
        output.left[frame] += left * voice.leftGain * voice.envelope;
        output.right[frame] += right * voice.rightGain * voice.envelope;
        voice.position += voice.increment;
        if (voice.releasing) voice.envelope *= voice.releaseMultiplier;
        else voice.envelope = std::min(1.0F, voice.envelope + voice.attackIncrement);
    }
}

void SamplePiano::stealVoiceIfNeeded() noexcept {
    if (voices_.size() < definition_.maximumVoices) return;
    const auto candidate = std::ranges::min_element(voices_, {}, [](const Voice& voice) {
        return voice.envelope + (voice.releasing ? 0.0F : 1.0F);
    });
    if (candidate != voices_.end()) voices_.erase(candidate);
}

} // namespace vll::audio
