#pragma once

#include "vll/midi/IMidiInput.h"
#include "vll/midi/MidiByteStreamParser.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace vll::midi {

class LinuxRawMidiInput final : public IMidiInput {
public:
    LinuxRawMidiInput() = default;
    ~LinuxRawMidiInput() override;

    LinuxRawMidiInput(const LinuxRawMidiInput&) = delete;
    LinuxRawMidiInput& operator=(const LinuxRawMidiInput&) = delete;

    [[nodiscard]] std::vector<DeviceInfo> devices() const override;
    bool connect(const std::string& deviceId) override;
    void disconnect() override;
    void setEventHandler(EventHandler handler) override;
    void setConnectionLostHandler(ConnectionLostHandler handler) override;
    [[nodiscard]] bool isConnected() const noexcept override;
    [[nodiscard]] TimestampMicros latencyMicros() const override;

private:
    void readLoop(std::stop_token stopToken);
    [[nodiscard]] static TimestampMicros nowMicros() noexcept;

    mutable std::mutex handlerMutex_;
    EventHandler handler_;
    ConnectionLostHandler connectionLostHandler_;
    MidiByteStreamParser parser_;
    std::jthread worker_;
    std::atomic<bool> connected_{false};
    int fileDescriptor_{-1};
};

} // namespace vll::midi
