#include "vll/midi/VirtualMidiInput.h"

#include <utility>

namespace vll::midi {

std::vector<DeviceInfo> VirtualMidiInput::devices() const {
    return {{deviceId(), "Virtual MIDI Test Device"}};
}

bool VirtualMidiInput::connect(const std::string& requestedDeviceId) {
    connected_ = requestedDeviceId == deviceId();
    if (connected_) parser_.reset();
    return connected_;
}

void VirtualMidiInput::disconnect() {
    connected_ = false;
    parser_.reset();
}

void VirtualMidiInput::setEventHandler(EventHandler handler) {
    handler_ = std::move(handler);
}

void VirtualMidiInput::setConnectionLostHandler(ConnectionLostHandler handler) {
    connectionLostHandler_ = std::move(handler);
}

bool VirtualMidiInput::isConnected() const noexcept { return connected_; }
TimestampMicros VirtualMidiInput::latencyMicros() const { return 0; }

void VirtualMidiInput::emitBytes(const std::span<const std::uint8_t> bytes,
                                 const TimestampMicros timestamp) {
    if (!connected_) return;
    for (const auto byte : bytes) {
        if (const auto event = parser_.feed(byte, timestamp); event && handler_) {
            handler_(*event);
        }
    }
}

void VirtualMidiInput::playFixture(const std::vector<MidiFixturePacket>& fixture) {
    for (const auto& packet : fixture) emitBytes(packet.bytes, packet.timestamp);
}

void VirtualMidiInput::simulateConnectionLoss() {
    if (!connected_) return;
    connected_ = false;
    parser_.reset();
    if (connectionLostHandler_) connectionLostHandler_();
}

} // namespace vll::midi
