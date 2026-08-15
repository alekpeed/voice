#pragma once

#include "vll/core/Types.h"
#include <vector>

namespace vll::performance {

class IPerformanceState {
public:
    virtual ~IPerformanceState() = default;
    virtual void ingest(const NoteEvent& event) = 0;
    [[nodiscard]] virtual std::vector<Pitch> heldNotes() const = 0;
    [[nodiscard]] virtual std::vector<Pitch> soundingNotes() const = 0;
    [[nodiscard]] virtual std::vector<Sonority> completedSonorities() const = 0;
    virtual void reset() = 0;
};

} // namespace vll::performance
