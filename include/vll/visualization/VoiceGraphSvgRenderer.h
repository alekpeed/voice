#pragma once

#include "vll/visualization/IVisualizationModel.h"

#include <string>

namespace vll::visualization {

struct VoiceGraphRenderOptions {
    int width{1200};
    int height{700};
    bool showMovementLabels{true};
};

class VoiceGraphSvgRenderer final {
public:
    [[nodiscard]] std::string render(
        const Frame& frame,
        VoiceGraphRenderOptions options = {}) const;
};

} // namespace vll::visualization
