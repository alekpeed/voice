#include "vll/visualization/VisualizationModel.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <utility>

namespace vll::visualization {

VisualizationModel::VisualizationModel(std::vector<VoicePath> paths) {
    setVoicePaths(std::move(paths));
}

void VisualizationModel::setVoicePaths(std::vector<VoicePath> paths) {
    std::lock_guard lock(mutex_);
    for (auto& path : paths) {
        std::ranges::sort(path.points, {}, &VoicePoint::timestamp);
    }
    std::ranges::sort(paths, [](const VoicePath& left, const VoicePath& right) {
        return left.voice.id < right.voice.id;
    });
    paths_ = std::move(paths);
    isolatedVoice_.reset();
    playing_ = false;
    updateRangeLocked();
    cursor_ = contentFrom_;
    visibleFrom_ = contentFrom_;
    visibleTo_ = contentTo_;
}

void VisualizationModel::setTimeline(std::vector<TimelineMarker> markers) {
    std::lock_guard lock(mutex_);
    std::ranges::sort(markers, {}, &TimelineMarker::timestamp);
    timeline_ = std::move(markers);
}

Frame VisualizationModel::frame() const {
    std::lock_guard lock(mutex_);
    Frame result;
    result.voicePaths = paths_;
    result.timeline = timeline_;
    result.isolatedVoice = isolatedVoice_;
    result.cursor = cursor_;
    result.visibleFrom = visibleFrom_;
    result.visibleTo = visibleTo_;
    result.playbackRate = playbackRate_;
    result.playing = playing_;

    for (const auto& path : paths_) {
        if (isolatedVoice_ && path.voice.id != *isolatedVoice_) continue;
        const auto next = std::ranges::upper_bound(path.points, cursor_, {}, &VoicePoint::timestamp);
        if (next == path.points.begin()) continue;
        const auto& point = *std::prev(next);
        result.highlightedPitches.push_back(point.pitch);
        result.keyboardKeys.push_back({point.pitch, path.voice.id, isolatedVoice_.has_value()});
    }
    std::ranges::sort(result.highlightedPitches);
    std::ranges::sort(result.keyboardKeys, {}, &KeyboardKeyState::pitch);
    return result;
}

void VisualizationModel::isolateVoice(const std::optional<VoiceId> voiceId) {
    std::lock_guard lock(mutex_);
    isolatedVoice_ = voiceId && hasVoiceLocked(*voiceId) ? voiceId : std::nullopt;
}

void VisualizationModel::setCursor(const TimestampMicros timestamp) {
    std::lock_guard lock(mutex_);
    cursor_ = std::clamp(timestamp, contentFrom_, contentTo_);
}

void VisualizationModel::setViewport(const TimestampMicros from, const TimestampMicros to) {
    std::lock_guard lock(mutex_);
    const auto ordered = std::minmax(from, to);
    visibleFrom_ = std::clamp(ordered.first, contentFrom_, contentTo_);
    visibleTo_ = std::clamp(ordered.second, contentFrom_, contentTo_);
    if (visibleFrom_ == visibleTo_ && contentFrom_ != contentTo_) {
        visibleFrom_ = contentFrom_;
        visibleTo_ = contentTo_;
    }
}

void VisualizationModel::resetViewport() {
    std::lock_guard lock(mutex_);
    visibleFrom_ = contentFrom_;
    visibleTo_ = contentTo_;
}

void VisualizationModel::setPlaybackRate(const double rate) {
    std::lock_guard lock(mutex_);
    playbackRate_ = std::clamp(rate, 0.1, 1.0);
}

void VisualizationModel::setPlaying(const bool playing) {
    std::lock_guard lock(mutex_);
    playing_ = playing && !paths_.empty() && cursor_ < contentTo_;
}

void VisualizationModel::advancePlayback(const TimestampMicros elapsedRealTime) {
    std::lock_guard lock(mutex_);
    if (!playing_ || elapsedRealTime <= 0) return;
    const auto advance = static_cast<TimestampMicros>(
        std::llround(static_cast<double>(elapsedRealTime) * playbackRate_));
    cursor_ = std::min(contentTo_, cursor_ + advance);
    if (cursor_ >= contentTo_) playing_ = false;
}

std::vector<VoicePlaybackEvent> VisualizationModel::playbackEvents(
    const TimestampMicros releaseTail) const {
    std::lock_guard lock(mutex_);
    std::vector<VoicePlaybackEvent> events;
    if (paths_.empty()) return events;

    for (const auto& path : paths_) {
        if (path.points.empty() || (isolatedVoice_ && path.voice.id != *isolatedVoice_)) continue;
        const auto scaledTime = [&](const TimestampMicros timestamp) {
            return static_cast<TimestampMicros>(std::llround(
                static_cast<double>(timestamp - contentFrom_) / playbackRate_));
        };
        events.push_back({NoteEventType::NoteOn, path.voice.id, path.points.front().pitch,
                          scaledTime(path.points.front().timestamp)});
        for (std::size_t index = 1; index < path.points.size(); ++index) {
            if (path.points[index].pitch == path.points[index - 1].pitch) continue;
            const auto time = scaledTime(path.points[index].timestamp);
            events.push_back({NoteEventType::NoteOff, path.voice.id, path.points[index - 1].pitch, time});
            events.push_back({NoteEventType::NoteOn, path.voice.id, path.points[index].pitch, time});
        }
        events.push_back({NoteEventType::NoteOff, path.voice.id, path.points.back().pitch,
                          scaledTime(path.points.back().timestamp) + std::max<TimestampMicros>(0, releaseTail)});
    }
    std::ranges::sort(events, [](const VoicePlaybackEvent& left, const VoicePlaybackEvent& right) {
        if (left.timestamp != right.timestamp) return left.timestamp < right.timestamp;
        if (left.type != right.type) return left.type == NoteEventType::NoteOff;
        return left.voiceId < right.voiceId;
    });
    return events;
}

void VisualizationModel::updateRangeLocked() {
    bool foundPoint = false;
    TimestampMicros minimum = std::numeric_limits<TimestampMicros>::max();
    TimestampMicros maximum = std::numeric_limits<TimestampMicros>::min();
    for (const auto& path : paths_) {
        for (const auto& point : path.points) {
            minimum = std::min(minimum, point.timestamp);
            maximum = std::max(maximum, point.timestamp);
            foundPoint = true;
        }
    }
    contentFrom_ = foundPoint ? minimum : 0;
    contentTo_ = foundPoint ? maximum : 0;
}

bool VisualizationModel::hasVoiceLocked(const VoiceId voiceId) const {
    return std::ranges::any_of(paths_, [voiceId](const VoicePath& path) {
        return path.voice.id == voiceId;
    });
}

} // namespace vll::visualization
