#pragma once

#include "vll/visualization/IVisualizationModel.h"

#include <mutex>
#include <vector>

namespace vll::visualization {

class VisualizationModel final : public IVisualizationModel {
public:
    explicit VisualizationModel(std::vector<VoicePath> paths = {});

    void setVoicePaths(std::vector<VoicePath> paths);
    void setTimeline(std::vector<TimelineMarker> markers);

    [[nodiscard]] Frame frame() const override;
    void isolateVoice(std::optional<VoiceId> voiceId) override;
    void setCursor(TimestampMicros timestamp) override;
    void setViewport(TimestampMicros from, TimestampMicros to) override;
    void resetViewport() override;
    void setPlaybackRate(double rate) override;
    void setPlaying(bool playing) override;
    void advancePlayback(TimestampMicros elapsedRealTime) override;
    [[nodiscard]] std::vector<VoicePlaybackEvent> playbackEvents(
        TimestampMicros releaseTail = 500'000) const override;

private:
    void updateRangeLocked();
    [[nodiscard]] bool hasVoiceLocked(VoiceId voiceId) const;

    mutable std::mutex mutex_;
    std::vector<VoicePath> paths_;
    std::vector<TimelineMarker> timeline_;
    std::optional<VoiceId> isolatedVoice_;
    TimestampMicros cursor_{0};
    TimestampMicros contentFrom_{0};
    TimestampMicros contentTo_{0};
    TimestampMicros visibleFrom_{0};
    TimestampMicros visibleTo_{0};
    double playbackRate_{1.0};
    bool playing_{false};
};

} // namespace vll::visualization
