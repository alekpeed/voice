#pragma once

#include <array>
#include <string>
#include <vector>

namespace vll::audio {

enum class VelocityCurveType { Linear, Soft, Hard, Custom };

class VelocityCurve {
public:
    static VelocityCurve linear();
    static VelocityCurve soft();
    static VelocityCurve hard();
    static VelocityCurve custom(std::array<float, 128> values);

    [[nodiscard]] float apply(float normalizedVelocity) const noexcept;
    [[nodiscard]] VelocityCurveType type() const noexcept { return type_; }
    [[nodiscard]] const std::array<float, 128>& table() const noexcept { return table_; }

private:
    VelocityCurve(VelocityCurveType type, std::array<float, 128> values);
    VelocityCurveType type_{VelocityCurveType::Linear};
    std::array<float, 128> table_{};
};

struct VelocityCalibration {
    VelocityCurve curve{VelocityCurve::linear()};
    float softMedian{0.0F};
    float mediumMedian{0.0F};
    float loudMedian{0.0F};
};

class VelocityCalibrator {
public:
    void addSoft(float velocity);
    void addMedium(float velocity);
    void addLoud(float velocity);
    [[nodiscard]] bool ready() const noexcept;
    [[nodiscard]] VelocityCalibration build() const;

private:
    static float median(std::vector<float> values);
    std::vector<float> soft_;
    std::vector<float> medium_;
    std::vector<float> loud_;
};

} // namespace vll::audio
