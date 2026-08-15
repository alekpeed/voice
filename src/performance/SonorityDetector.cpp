#include "vll/performance/SonorityDetector.h"

#include <algorithm>
#include <iterator>

namespace vll::performance {

SonorityDetector::SonorityDetector(SonorityDetectorConfig config) : config_(config) {
    config_.chordWindowMicros = std::max<TimestampMicros>(0, config_.chordWindowMicros);
    config_.minimumNotes = std::max<std::size_t>(1, config_.minimumNotes);
    config_.historyLimit = std::max<std::size_t>(1, config_.historyLimit);
}

void SonorityDetector::ingest(const NoteEvent& incoming) {
    std::lock_guard lock(mutex_);
    NoteEvent event = incoming;
    event.timestamp = std::max(event.timestamp, lastTimestamp_);
    advanceTimeLocked(event.timestamp);

    const Key key{event.channel, event.pitch.midiNote};
    if (event.type == NoteEventType::NoteOn) {
        if (!pending_) {
            pending_ = true;
            pendingStartedAt_ = event.timestamp;
        }
        heldKeys_.insert(key);
        sustainedKeys_.erase(key);
        pendingLastEventAt_ = event.timestamp;
    } else if (event.type == NoteEventType::NoteOff) {
        heldKeys_.erase(key);
        if (sustainChannels_.contains(event.channel)) {
            sustainedKeys_.insert(key);
        } else {
            sustainedKeys_.erase(key);
        }
        if (pending_) pendingLastEventAt_ = event.timestamp;
    } else if (event.type == NoteEventType::SustainOn) {
        sustainChannels_.insert(event.channel);
        if (pending_) pendingLastEventAt_ = event.timestamp;
    } else if (event.type == NoteEventType::SustainOff) {
        sustainChannels_.erase(event.channel);
        std::erase_if(sustainedKeys_, [&](const Key& sustained) {
            return sustained.channel == event.channel && !heldKeys_.contains(sustained);
        });
        if (pending_) pendingLastEventAt_ = event.timestamp;
    }
    lastTimestamp_ = event.timestamp;
}

std::vector<Pitch> SonorityDetector::heldNotes() const {
    std::lock_guard lock(mutex_);
    return notesFromKeysLocked(false);
}

std::vector<Pitch> SonorityDetector::soundingNotes() const {
    std::lock_guard lock(mutex_);
    return notesFromKeysLocked(true);
}

std::vector<Sonority> SonorityDetector::completedSonorities() const {
    std::lock_guard lock(mutex_);
    return completed_;
}

void SonorityDetector::reset() {
    std::lock_guard lock(mutex_);
    heldKeys_.clear();
    sustainedKeys_.clear();
    sustainChannels_.clear();
    completed_.clear();
    pending_ = false;
    pendingStartedAt_ = 0;
    pendingLastEventAt_ = 0;
    lastTimestamp_ = 0;
}

void SonorityDetector::advanceTime(const TimestampMicros timestamp) {
    std::lock_guard lock(mutex_);
    advanceTimeLocked(std::max(timestamp, lastTimestamp_));
    lastTimestamp_ = std::max(timestamp, lastTimestamp_);
}

SonorityDetectorConfig SonorityDetector::config() const noexcept { return config_; }

void SonorityDetector::advanceTimeLocked(const TimestampMicros timestamp) {
    if (pending_ && timestamp - pendingStartedAt_ >= config_.chordWindowMicros) {
        completePendingLocked();
    }
}

void SonorityDetector::completePendingLocked() {
    auto pitches = notesFromKeysLocked(true);
    if (pitches.size() >= config_.minimumNotes) {
        completed_.push_back(Sonority{std::move(pitches), pendingStartedAt_, pendingLastEventAt_});
        if (completed_.size() > config_.historyLimit) {
            completed_.erase(completed_.begin(),
                             std::next(completed_.begin(),
                                       static_cast<std::ptrdiff_t>(completed_.size() - config_.historyLimit)));
        }
    }
    pending_ = false;
}

std::vector<Pitch> SonorityDetector::notesFromKeysLocked(const bool sounding) const {
    std::set<int> midiNotes;
    for (const auto& key : heldKeys_) midiNotes.insert(key.midiNote);
    if (sounding) {
        for (const auto& key : sustainedKeys_) midiNotes.insert(key.midiNote);
    }

    std::vector<Pitch> pitches;
    pitches.reserve(midiNotes.size());
    std::ranges::transform(midiNotes, std::back_inserter(pitches),
                           [](const int midiNote) { return Pitch{midiNote}; });
    return pitches;
}

} // namespace vll::performance
