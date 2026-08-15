#include "vll/midi/LinuxRawMidiInput.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <filesystem>
#include <string_view>
#include <utility>

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#endif

namespace vll::midi {
namespace {

bool isLinuxRawMidiName(const std::string_view name) noexcept {
    constexpr std::string_view prefix{"midiC"};
    if (!name.starts_with(prefix)) return false;
    std::size_t position = prefix.size();
    const std::size_t cardStart = position;
    while (position < name.size() && name[position] >= '0' && name[position] <= '9') ++position;
    if (position == cardStart || position >= name.size() || name[position] != 'D') return false;
    ++position;
    const std::size_t deviceStart = position;
    while (position < name.size() && name[position] >= '0' && name[position] <= '9') ++position;
    return position > deviceStart && position == name.size();
}

} // namespace

LinuxRawMidiInput::~LinuxRawMidiInput() { disconnect(); }

std::vector<DeviceInfo> LinuxRawMidiInput::devices() const {
    std::vector<DeviceInfo> result;
#ifdef __linux__
    const std::filesystem::path soundDevices{"/dev/snd"};
    if (!std::filesystem::exists(soundDevices)) return result;

    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(soundDevices, error)) {
        if (error || !entry.is_character_file(error)) continue;
        const auto filename = entry.path().filename().string();
        if (isLinuxRawMidiName(filename)) {
            result.push_back({entry.path().string(), "Linux Raw MIDI " + filename});
        }
    }
    std::ranges::sort(result, {}, &DeviceInfo::id);
#endif
    return result;
}

bool LinuxRawMidiInput::connect(const std::string& deviceId) {
    disconnect();
#ifdef __linux__
    const auto available = devices();
    const auto found = std::ranges::find(available, deviceId, &DeviceInfo::id);
    if (found == available.end()) return false;

    fileDescriptor_ = ::open(deviceId.c_str(), O_RDONLY | O_NONBLOCK);
    if (fileDescriptor_ < 0) return false;
    parser_.reset();
    connected_.store(true);
    worker_ = std::jthread([this](const std::stop_token token) { readLoop(token); });
    return true;
#else
    static_cast<void>(deviceId);
    return false;
#endif
}

void LinuxRawMidiInput::disconnect() {
    connected_.store(false);
    if (worker_.joinable()) {
        worker_.request_stop();
        worker_.join();
    }
#ifdef __linux__
    if (fileDescriptor_ >= 0) {
        ::close(fileDescriptor_);
        fileDescriptor_ = -1;
    }
#endif
    parser_.reset();
}

void LinuxRawMidiInput::setEventHandler(EventHandler handler) {
    std::lock_guard lock(handlerMutex_);
    handler_ = std::move(handler);
}

void LinuxRawMidiInput::setConnectionLostHandler(ConnectionLostHandler handler) {
    std::lock_guard lock(handlerMutex_);
    connectionLostHandler_ = std::move(handler);
}

bool LinuxRawMidiInput::isConnected() const noexcept { return connected_.load(); }
TimestampMicros LinuxRawMidiInput::latencyMicros() const { return 0; }

void LinuxRawMidiInput::readLoop(const std::stop_token stopToken) {
#ifdef __linux__
    std::array<std::uint8_t, 256> buffer{};
    while (!stopToken.stop_requested()) {
        const auto bytesRead = ::read(fileDescriptor_, buffer.data(), buffer.size());
        if (bytesRead > 0) {
            for (std::ptrdiff_t index = 0; index < bytesRead; ++index) {
                if (const auto event = parser_.feed(buffer[static_cast<std::size_t>(index)],
                                                    nowMicros())) {
                    EventHandler handler;
                    {
                        std::lock_guard lock(handlerMutex_);
                        handler = handler_;
                    }
                    if (handler) handler(*event);
                }
            }
        } else if (bytesRead == 0 || errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            connected_.store(false);
            ConnectionLostHandler connectionLostHandler;
            {
                std::lock_guard lock(handlerMutex_);
                connectionLostHandler = connectionLostHandler_;
            }
            if (connectionLostHandler) connectionLostHandler();
            break;
        }
    }
#else
    static_cast<void>(stopToken);
#endif
}

TimestampMicros LinuxRawMidiInput::nowMicros() noexcept {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

} // namespace vll::midi
