#pragma once

#include "vll/audio/IInstrument.h"

#include <atomic>
#include <cstddef>
#include <string>
#include <thread>

namespace vll::audio {

class LinuxAlsaOutput {
public:
    LinuxAlsaOutput() = default;
    ~LinuxAlsaOutput();

    LinuxAlsaOutput(const LinuxAlsaOutput&) = delete;
    LinuxAlsaOutput& operator=(const LinuxAlsaOutput&) = delete;

    bool start(IInstrument& instrument, const std::string& device = "default",
               unsigned int sampleRate = 48000, std::size_t blockSize = 128,
               unsigned int requestedLatencyMicros = 10000);
    void stop();
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] unsigned int requestedLatencyMicros() const noexcept;
    [[nodiscard]] const std::string& lastError() const noexcept;

private:
    void renderLoop(std::stop_token stopToken);
    bool loadLibrary();
    void unloadLibrary();

    struct Api;
    Api* api_{nullptr};
    void* library_{nullptr};
    void* pcm_{nullptr};
    IInstrument* instrument_{nullptr};
    std::jthread worker_;
    std::atomic<bool> running_{false};
    unsigned int sampleRate_{48000};
    std::size_t blockSize_{128};
    unsigned int latencyMicros_{10000};
    std::string lastError_;
};

} // namespace vll::audio
