#pragma once

#include "vll/midi/IMidiInput.h"

#include <deque>
#include <functional>
#include <mutex>
#include <set>
#include <vector>

namespace vll::midi {

struct ActiveNote {
    int channel{1};
    Pitch pitch{};
    auto operator<=>(const ActiveNote&) const = default;
};

struct MonitoredEvent {
    NoteEvent event;
    bool synthetic{false};
};

class MidiSession {
public:
    using EventHandler = std::function<void(const MonitoredEvent&)>;

    explicit MidiSession(IMidiInput& input, std::size_t monitorCapacity = 512);
    ~MidiSession();

    MidiSession(const MidiSession&) = delete;
    MidiSession& operator=(const MidiSession&) = delete;

    bool connect(const std::string& deviceId);
    void disconnect();
    void setEventHandler(EventHandler handler);

    [[nodiscard]] bool isConnected() const noexcept;
    [[nodiscard]] std::vector<ActiveNote> activeNotes() const;
    [[nodiscard]] bool sustainDown() const;
    [[nodiscard]] std::vector<MonitoredEvent> recentEvents() const;

private:
    void receive(NoteEvent event);
    void publish(const MonitoredEvent& event);
    void releaseActiveState();

    IMidiInput& input_;
    const std::size_t monitorCapacity_;
    mutable std::mutex mutex_;
    std::set<ActiveNote> activeNotes_;
    std::deque<MonitoredEvent> monitor_;
    EventHandler handler_;
    bool sustainDown_{false};
    TimestampMicros lastTimestamp_{0};
};

} // namespace vll::midi
