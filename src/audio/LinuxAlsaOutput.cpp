#include "vll/audio/LinuxAlsaOutput.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#ifdef __linux__
#include <dlfcn.h>
#endif

namespace vll::audio {

struct LinuxAlsaOutput::Api {
    using Open = int (*)(void**, const char*, int, int);
    using SetParams = int (*)(void*, int, int, unsigned int, unsigned int, int, unsigned int);
    using WriteInterleaved = long (*)(void*, const void*, unsigned long);
    using Recover = int (*)(void*, int, int);
    using Drop = int (*)(void*);
    using Close = int (*)(void*);
    using ErrorText = const char* (*)(int);

    Open open{nullptr};
    SetParams setParams{nullptr};
    WriteInterleaved writeInterleaved{nullptr};
    Recover recover{nullptr};
    Drop drop{nullptr};
    Close close{nullptr};
    ErrorText errorText{nullptr};
};

namespace {

#ifdef __linux__
template <typename Function>
bool loadFunction(void* library, const char* name, Function& destination) {
    void* symbol = ::dlsym(library, name);
    if (symbol == nullptr || sizeof(symbol) != sizeof(destination)) return false;
    std::memcpy(&destination, &symbol, sizeof(destination));
    return true;
}
#endif

} // namespace

LinuxAlsaOutput::~LinuxAlsaOutput() { stop(); }

bool LinuxAlsaOutput::start(IInstrument& instrument, const std::string& device,
                            const unsigned int sampleRate, const std::size_t blockSize,
                            const unsigned int requestedLatencyMicros) {
    stop();
    lastError_.clear();
    sampleRate_ = std::clamp(sampleRate, 8000U, 384000U);
    blockSize_ = std::clamp<std::size_t>(blockSize, 16, 4096);
    latencyMicros_ = std::clamp(requestedLatencyMicros, 2000U, 500000U);
    if (!loadLibrary()) return false;

    constexpr int playbackStream = 0;
    constexpr int nonBlockingOpen = 0;
    int status = api_->open(&pcm_, device.c_str(), playbackStream, nonBlockingOpen);
    if (status < 0) {
        lastError_ = std::string("Unable to open ALSA device: ") + api_->errorText(status);
        unloadLibrary();
        return false;
    }

    constexpr int signed16LittleEndian = 2;
    constexpr int readWriteInterleaved = 3;
    status = api_->setParams(pcm_, signed16LittleEndian, readWriteInterleaved, 2,
                             sampleRate_, 1, latencyMicros_);
    if (status < 0) {
        lastError_ = std::string("Unable to configure ALSA device: ") + api_->errorText(status);
        api_->close(pcm_);
        pcm_ = nullptr;
        unloadLibrary();
        return false;
    }

    instrument_ = &instrument;
    instrument_->prepare(sampleRate_, blockSize_);
    running_.store(true);
    worker_ = std::jthread([this](const std::stop_token token) { renderLoop(token); });
    return true;
}

void LinuxAlsaOutput::stop() {
    running_.store(false);
    if (worker_.joinable()) {
        worker_.request_stop();
        worker_.join();
    }
    if (pcm_ != nullptr && api_ != nullptr) {
        api_->drop(pcm_);
        api_->close(pcm_);
        pcm_ = nullptr;
    }
    instrument_ = nullptr;
    unloadLibrary();
}

bool LinuxAlsaOutput::isRunning() const noexcept { return running_.load(); }
unsigned int LinuxAlsaOutput::requestedLatencyMicros() const noexcept { return latencyMicros_; }
const std::string& LinuxAlsaOutput::lastError() const noexcept { return lastError_; }

void LinuxAlsaOutput::renderLoop(const std::stop_token stopToken) {
    std::vector<float> left(blockSize_);
    std::vector<float> right(blockSize_);
    std::vector<std::int16_t> interleaved(blockSize_ * 2);
    while (!stopToken.stop_requested()) {
        instrument_->renderAudio({left, right});
        for (std::size_t frame = 0; frame < blockSize_; ++frame) {
            const auto leftValue = static_cast<int>(std::lround(
                std::clamp(left[frame], -1.0F, 1.0F) * 32767.0F));
            const auto rightValue = static_cast<int>(std::lround(
                std::clamp(right[frame], -1.0F, 1.0F) * 32767.0F));
            interleaved[frame * 2] = static_cast<std::int16_t>(leftValue);
            interleaved[frame * 2 + 1] = static_cast<std::int16_t>(rightValue);
        }

        std::size_t written = 0;
        while (written < blockSize_ && !stopToken.stop_requested()) {
            const long result = api_->writeInterleaved(
                pcm_, interleaved.data() + written * 2,
                static_cast<unsigned long>(blockSize_ - written));
            if (result < 0) {
                if (api_->recover(pcm_, static_cast<int>(result), 1) < 0) {
                    running_.store(false);
                    return;
                }
            } else if (result > 0) {
                written += static_cast<std::size_t>(result);
            } else {
                std::this_thread::yield();
            }
        }
    }
    running_.store(false);
}

bool LinuxAlsaOutput::loadLibrary() {
#ifdef __linux__
    library_ = ::dlopen("libasound.so.2", RTLD_NOW | RTLD_LOCAL);
    if (library_ == nullptr) {
        lastError_ = "libasound.so.2 is unavailable";
        return false;
    }
    api_ = new Api{};
    const bool loaded = loadFunction(library_, "snd_pcm_open", api_->open) &&
                        loadFunction(library_, "snd_pcm_set_params", api_->setParams) &&
                        loadFunction(library_, "snd_pcm_writei", api_->writeInterleaved) &&
                        loadFunction(library_, "snd_pcm_recover", api_->recover) &&
                        loadFunction(library_, "snd_pcm_drop", api_->drop) &&
                        loadFunction(library_, "snd_pcm_close", api_->close) &&
                        loadFunction(library_, "snd_strerror", api_->errorText);
    if (!loaded) {
        lastError_ = "The installed ALSA library is missing required PCM functions";
        unloadLibrary();
        return false;
    }
    return true;
#else
    lastError_ = "ALSA output is available only on Linux";
    return false;
#endif
}

void LinuxAlsaOutput::unloadLibrary() {
    delete api_;
    api_ = nullptr;
#ifdef __linux__
    if (library_ != nullptr) ::dlclose(library_);
#endif
    library_ = nullptr;
}

} // namespace vll::audio
