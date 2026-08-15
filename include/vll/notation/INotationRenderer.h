#pragma once

#include "vll/core/Types.h"
#include <string>
#include <vector>

namespace vll::notation {

struct EngravingOptions {
    float scale{1.0F};
    bool showChordSymbols{true};
    bool showAnalysisMarks{true};
    float maximumSystemWidth{960.0F};
};

struct RenderedNotation {
    std::string scalableVectorData;
    float width{0.0F};
    float height{0.0F};
};

class INotationRenderer {
public:
    virtual ~INotationRenderer() = default;
    [[nodiscard]] virtual RenderedNotation render(const std::vector<Sonority>& sonorities,
                                                   const EngravingOptions& options) const = 0;
};

} // namespace vll::notation
