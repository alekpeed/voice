#pragma once

#include "vll/audio/IInstrument.h"
#include "vll/core/Types.h"

namespace vll::audio {

class MidiAudioRouter {
public:
    explicit MidiAudioRouter(IInstrument& instrument) : instrument_(instrument) {}
    void process(const NoteEvent& event) noexcept;

private:
    IInstrument& instrument_;
};

} // namespace vll::audio
