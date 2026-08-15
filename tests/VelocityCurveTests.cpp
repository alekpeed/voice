#include "TestFramework.h"
#include "vll/audio/VelocityCurve.h"

TEST_CASE("factory velocity curves preserve endpoints") {
    const auto linear = vll::audio::VelocityCurve::linear();
    const auto soft = vll::audio::VelocityCurve::soft();
    const auto hard = vll::audio::VelocityCurve::hard();
    REQUIRE_EQ(linear.apply(0.0F), 0.0F);
    REQUIRE_EQ(linear.apply(1.0F), 1.0F);
    REQUIRE(soft.apply(0.4F) > linear.apply(0.4F));
    REQUIRE(hard.apply(0.4F) < linear.apply(0.4F));
}

TEST_CASE("velocity calibration derives an ordered custom curve") {
    vll::audio::VelocityCalibrator calibrator;
    for (const float value : {0.10F, 0.12F, 0.14F}) calibrator.addSoft(value);
    for (const float value : {0.38F, 0.40F, 0.42F}) calibrator.addMedium(value);
    for (const float value : {0.72F, 0.75F, 0.78F}) calibrator.addLoud(value);
    REQUIRE(calibrator.ready());
    const auto calibration = calibrator.build();
    REQUIRE_EQ(calibration.curve.type(), vll::audio::VelocityCurveType::Custom);
    REQUIRE(calibration.curve.apply(calibration.softMedian) > 0.18F);
    REQUIRE(calibration.curve.apply(calibration.mediumMedian) > 0.52F);
    REQUIRE(calibration.curve.apply(calibration.loudMedian) > 0.92F);
}

TEST_CASE("custom velocity curves are forced monotonic") {
    std::array<float, 128> values{};
    values[0] = 0.2F;
    values[1] = 0.1F;
    const auto curve = vll::audio::VelocityCurve::custom(values);
    REQUIRE(curve.table()[1] >= curve.table()[0]);
}
