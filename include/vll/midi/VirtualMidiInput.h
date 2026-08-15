#pragma once

#include "vll/midi/IMidiInput.h"
#include "vll/midi/MidiByteStreamParser.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace vll::midi {

struct MidiFixturePacket {
    TimestampMicros timestamp{0};
    std::vector<std::uint8_t> bytes;
};

class VirtualMidiInput final : public IMidiInput {
public:
    [[nodiscard]] std::vector<DeviceInfo> devices() const override;
    bool connect(const std::string& deviceId) override;
    void disconnect() override;
    void setEventHandler(EventHandler handler) override;
    void setConnectionLostHandler(ConnectionLostHandler handler) override;
    [[nodiscard]] bool isConnected() const noexcept override;
    [[nodiscard]] TimestampMicros latencyMicros() const override;

    void emitBytes(std::span<const std::uint8_t> bytes, TimestampMicros timestamp);
    void playFixture(const std::vector<MidiFixturePacket>& fixture);
    void simulateConnectionLoss();

    static constexpr const char* deviceId() noexcept { return "virtual-midi-0"; }

private:
    EventHandler handler_;
    ConnectionLostHandler connectionLostHandler_;
    MidiByteStreamParser parser_;
    bool connected_{false};
};

} // namespace vll::midi
