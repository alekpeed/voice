#include "vll/audio/MidiAudioRouter.h"

namespace vll::audio {

void MidiAudioRouter::process(const NoteEvent& event) noexcept {
    switch (event.type) {
        case NoteEventType::NoteOn:
            instrument_.noteOn(event.pitch, event.velocity);
            break;
        case NoteEventType::NoteOff:
            instrument_.noteOff(event.pitch);
            break;
        case NoteEventType::SustainOn:
            instrument_.pedal(event.velocity);
            break;
        case NoteEventType::SustainOff:
            instrument_.pedal(0.0F);
            break;
    }
}

} // namespace vll::audio
