#pragma once

#include "vll/core/Types.h"

#include <array>
#include <cstdint>
#include <optional>

namespace vll::midi {

class MidiByteStreamParser {
public:
    [[nodiscard]] std::optional<NoteEvent> feed(std::uint8_t byte,
                                                TimestampMicros timestamp) noexcept;
    void reset() noexcept;

private:
    [[nodiscard]] int expectedDataBytes() const noexcept;
    [[nodiscard]] std::optional<NoteEvent> makeEvent(TimestampMicros timestamp) const noexcept;

    std::uint8_t runningStatus_{0};
    std::array<std::uint8_t, 2> data_{};
    int dataCount_{0};
};

} // namespace vll::midi
