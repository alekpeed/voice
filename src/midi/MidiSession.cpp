#include "vll/midi/MidiSession.h"

#include <algorithm>
#include <utility>

namespace vll::midi {

MidiSession::MidiSession(IMidiInput& input, const std::size_t monitorCapacity)
    : input_(input), monitorCapacity_(std::max<std::size_t>(1, monitorCapacity)) {
    input_.setEventHandler([this](const NoteEvent& event) { receive(event); });
    input_.setConnectionLostHandler([this] { releaseActiveState(); });
}

MidiSession::~MidiSession() {
    disconnect();
    input_.setEventHandler({});
    input_.setConnectionLostHandler({});
}

bool MidiSession::connect(const std::string& deviceId) {
    if (input_.isConnected()) disconnect();
    {
        std::lock_guard lock(mutex_);
        lastTimestamp_ = 0;
    }
    return input_.connect(deviceId);
}

void MidiSession::disconnect() {
    input_.disconnect();
    releaseActiveState();
}

void MidiSession::releaseActiveState() {
    std::vector<MonitoredEvent> releases;
    {
        std::lock_guard lock(mutex_);
        TimestampMicros timestamp = lastTimestamp_ + 1;
        for (const auto& note : activeNotes_) {
            releases.push_back({NoteEvent{NoteEventType::NoteOff, note.pitch, 0.0F,
                                                timestamp++, note.channel}, true});
        }
        if (sustainDown_) {
            releases.push_back({NoteEvent{NoteEventType::SustainOff, Pitch{0}, 0.0F,
                                                timestamp, 1}, true});
        }
        activeNotes_.clear();
        sustainDown_ = false;
    }

    for (const auto& release : releases) publish(release);
}

void MidiSession::setEventHandler(EventHandler handler) {
    std::lock_guard lock(mutex_);
    handler_ = std::move(handler);
}

bool MidiSession::isConnected() const noexcept { return input_.isConnected(); }

std::vector<ActiveNote> MidiSession::activeNotes() const {
    std::lock_guard lock(mutex_);
    return {activeNotes_.begin(), activeNotes_.end()};
}

bool MidiSession::sustainDown() const {
    std::lock_guard lock(mutex_);
    return sustainDown_;
}

std::vector<MonitoredEvent> MidiSession::recentEvents() const {
    std::lock_guard lock(mutex_);
    return {monitor_.begin(), monitor_.end()};
}

void MidiSession::receive(NoteEvent event) {
    std::vector<MonitoredEvent> events;
    events.reserve(2);
    {
        std::lock_guard lock(mutex_);
        event.timestamp = std::max(event.timestamp, lastTimestamp_);
        lastTimestamp_ = event.timestamp;

        const ActiveNote key{event.channel, event.pitch};
        if (event.type == NoteEventType::NoteOn) {
            if (activeNotes_.contains(key)) {
                events.push_back({NoteEvent{NoteEventType::NoteOff, event.pitch, 0.0F,
                                                  event.timestamp, event.channel}, true});
            }
            activeNotes_.insert(key);
            events.push_back({event, false});
        } else if (event.type == NoteEventType::NoteOff) {
            if (activeNotes_.erase(key) > 0) events.push_back({event, false});
        } else if (event.type == NoteEventType::SustainOn) {
            sustainDown_ = true;
            events.push_back({event, false});
        } else if (event.type == NoteEventType::SustainOff) {
            sustainDown_ = false;
            events.push_back({event, false});
        }
    }

    for (const auto& monitored : events) publish(monitored);
}

void MidiSession::publish(const MonitoredEvent& event) {
    EventHandler handler;
    {
        std::lock_guard lock(mutex_);
        monitor_.push_back(event);
        while (monitor_.size() > monitorCapacity_) monitor_.pop_front();
        handler = handler_;
        lastTimestamp_ = std::max(lastTimestamp_, event.event.timestamp);
    }
    if (handler) handler(event);
}

} // namespace vll::midi
