#pragma once

#include "vll/performance/IPerformanceState.h"

#include <cstddef>
#include <mutex>
#include <set>
#include <vector>

namespace vll::performance {

struct SonorityDetectorConfig {
    TimestampMicros chordWindowMicros{80'000};
    std::size_t minimumNotes{2};
    std::size_t historyLimit{256};
};

class SonorityDetector final : public IPerformanceState {
public:
    explicit SonorityDetector(SonorityDetectorConfig config = {});

    void ingest(const NoteEvent& event) override;
    [[nodiscard]] std::vector<Pitch> heldNotes() const override;
    [[nodiscard]] std::vector<Pitch> soundingNotes() const override;
    [[nodiscard]] std::vector<Sonority> completedSonorities() const override;
    void reset() override;

    void advanceTime(TimestampMicros timestamp);
    [[nodiscard]] SonorityDetectorConfig config() const noexcept;

private:
    struct Key {
        int channel{1};
        int midiNote{60};
        auto operator<=>(const Key&) const = default;
    };

    void advanceTimeLocked(TimestampMicros timestamp);
    void completePendingLocked();
    [[nodiscard]] std::vector<Pitch> notesFromKeysLocked(bool sounding) const;

    SonorityDetectorConfig config_;
    mutable std::mutex mutex_;
    std::set<Key> heldKeys_;
    std::set<Key> sustainedKeys_;
    std::set<int> sustainChannels_;
    std::vector<Sonority> completed_;
    bool pending_{false};
    TimestampMicros pendingStartedAt_{0};
    TimestampMicros pendingLastEventAt_{0};
    TimestampMicros lastTimestamp_{0};
};

} // namespace vll::performance
