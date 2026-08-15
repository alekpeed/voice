#include "vll/audio/VelocityCurve.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace vll::audio {
namespace {

std::array<float, 128> gammaTable(const float gamma) {
    std::array<float, 128> values{};
    for (std::size_t index = 0; index < values.size(); ++index) {
        values[index] = std::pow(static_cast<float>(index) / 127.0F, gamma);
    }
    return values;
}

float interpolate(const float x, const float x0, const float y0,
                  const float x1, const float y1) noexcept {
    if (x1 <= x0) return y1;
    const float amount = std::clamp((x - x0) / (x1 - x0), 0.0F, 1.0F);
    return y0 + amount * (y1 - y0);
}

} // namespace

VelocityCurve::VelocityCurve(const VelocityCurveType type, std::array<float, 128> values)
    : type_(type), table_(std::move(values)) {}

VelocityCurve VelocityCurve::linear() { return {VelocityCurveType::Linear, gammaTable(1.0F)}; }
VelocityCurve VelocityCurve::soft() { return {VelocityCurveType::Soft, gammaTable(0.72F)}; }
VelocityCurve VelocityCurve::hard() { return {VelocityCurveType::Hard, gammaTable(1.45F)}; }

VelocityCurve VelocityCurve::custom(std::array<float, 128> values) {
    values.front() = std::clamp(values.front(), 0.0F, 1.0F);
    for (std::size_t index = 1; index < values.size(); ++index) {
        values[index] = std::clamp(values[index], values[index - 1], 1.0F);
    }
    return {VelocityCurveType::Custom, std::move(values)};
}

float VelocityCurve::apply(const float normalizedVelocity) const noexcept {
    const float scaled = std::clamp(normalizedVelocity, 0.0F, 1.0F) * 127.0F;
    const auto lower = static_cast<std::size_t>(scaled);
    const auto upper = std::min<std::size_t>(127, lower + 1);
    const float fraction = scaled - static_cast<float>(lower);
    return table_[lower] + fraction * (table_[upper] - table_[lower]);
}

void VelocityCalibrator::addSoft(const float velocity) { soft_.push_back(std::clamp(velocity, 0.0F, 1.0F)); }
void VelocityCalibrator::addMedium(const float velocity) { medium_.push_back(std::clamp(velocity, 0.0F, 1.0F)); }
void VelocityCalibrator::addLoud(const float velocity) { loud_.push_back(std::clamp(velocity, 0.0F, 1.0F)); }

bool VelocityCalibrator::ready() const noexcept {
    return soft_.size() >= 3 && medium_.size() >= 3 && loud_.size() >= 3;
}

VelocityCalibration VelocityCalibrator::build() const {
    if (!ready()) throw std::logic_error("Velocity calibration needs at least three samples per level");
    const float soft = median(soft_);
    const float medium = median(medium_);
    const float loud = median(loud_);
    if (!(soft < medium && medium < loud)) {
        throw std::logic_error("Soft, medium, and loud velocity medians must be ordered");
    }

    std::array<float, 128> table{};
    for (std::size_t index = 0; index < table.size(); ++index) {
        const float input = static_cast<float>(index) / 127.0F;
        if (input <= soft) table[index] = interpolate(input, 0.0F, 0.0F, soft, 0.20F);
        else if (input <= medium) table[index] = interpolate(input, soft, 0.20F, medium, 0.55F);
        else if (input <= loud) table[index] = interpolate(input, medium, 0.55F, loud, 0.95F);
        else table[index] = interpolate(input, loud, 0.95F, 1.0F, 1.0F);
    }
    return {VelocityCurve::custom(table), soft, medium, loud};
}

float VelocityCalibrator::median(std::vector<float> values) {
    std::ranges::sort(values);
    const auto middle = values.size() / 2;
    return values.size() % 2 == 0 ? (values[middle - 1] + values[middle]) * 0.5F
                                  : values[middle];
}

} // namespace vll::audio
