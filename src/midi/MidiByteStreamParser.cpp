#include "vll/midi/MidiByteStreamParser.h"

namespace vll::midi {

std::optional<NoteEvent> MidiByteStreamParser::feed(const std::uint8_t byte,
                                                     const TimestampMicros timestamp) noexcept {
    if (byte >= 0xF8) return std::nullopt;

    if ((byte & 0x80U) != 0U) {
        dataCount_ = 0;
        if (byte >= 0xF0) {
            runningStatus_ = 0;
        } else {
            runningStatus_ = byte;
        }
        return std::nullopt;
    }

    if (runningStatus_ == 0) return std::nullopt;
    const int required = expectedDataBytes();
    if (required == 0) return std::nullopt;

    data_[static_cast<std::size_t>(dataCount_++)] = byte;
    if (dataCount_ < required) return std::nullopt;

    dataCount_ = 0;
    return makeEvent(timestamp);
}

void MidiByteStreamParser::reset() noexcept {
    runningStatus_ = 0;
    dataCount_ = 0;
    data_ = {};
}

int MidiByteStreamParser::expectedDataBytes() const noexcept {
    const auto message = static_cast<std::uint8_t>(runningStatus_ & 0xF0U);
    return (message == 0xC0U || message == 0xD0U) ? 1 : 2;
}

std::optional<NoteEvent> MidiByteStreamParser::makeEvent(
    const TimestampMicros timestamp) const noexcept {
    const auto message = static_cast<std::uint8_t>(runningStatus_ & 0xF0U);
    const int channel = static_cast<int>(runningStatus_ & 0x0FU) + 1;

    if (message == 0x80U || message == 0x90U) {
        const bool noteOff = message == 0x80U || data_[1] == 0;
        return NoteEvent{
            noteOff ? NoteEventType::NoteOff : NoteEventType::NoteOn,
            Pitch{static_cast<int>(data_[0])},
            noteOff ? 0.0F : static_cast<float>(data_[1]) / 127.0F,
            timestamp,
            channel
        };
    }

    if (message == 0xB0U && data_[0] == 64) {
        return NoteEvent{
            data_[1] >= 64 ? NoteEventType::SustainOn : NoteEventType::SustainOff,
            Pitch{0},
            static_cast<float>(data_[1]) / 127.0F,
            timestamp,
            channel
        };
    }

    return std::nullopt;
}

} // namespace vll::midi
