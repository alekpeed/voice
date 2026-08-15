#pragma once

#include "vll/notation/INotationRenderer.h"
#include "vll/notation/NotationLayout.h"

namespace vll::notation {

class SvgNotationRenderer final : public INotationRenderer {
public:
    [[nodiscard]] RenderedNotation render(
        const NotationDocument& document,
        const EngravingOptions& options) const override;

    [[nodiscard]] NotationLayout layout(
        const NotationDocument& document,
        const EngravingOptions& options) const;
};

} // namespace vll::notation
