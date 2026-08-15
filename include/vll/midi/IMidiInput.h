#pragma once

#include "vll/core/Types.h"
#include <functional>
#include <string>
#include <vector>

namespace vll::midi {

struct DeviceInfo { std::string id; std::string name; };

class IMidiInput {
public:
    using EventHandler = std::function<void(const NoteEvent&)>;
    using ConnectionLostHandler = std::function<void()>;
    virtual ~IMidiInput() = default;
    [[nodiscard]] virtual std::vector<DeviceInfo> devices() const = 0;
    virtual bool connect(const std::string& deviceId) = 0;
    virtual void disconnect() = 0;
    virtual void setEventHandler(EventHandler handler) = 0;
    virtual void setConnectionLostHandler(ConnectionLostHandler handler) = 0;
    [[nodiscard]] virtual bool isConnected() const noexcept = 0;
    [[nodiscard]] virtual TimestampMicros latencyMicros() const = 0;
};

} // namespace vll::midi
