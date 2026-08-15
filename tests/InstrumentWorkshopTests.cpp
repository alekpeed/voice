#include "TestFramework.h"
#include "vll/audio/InstrumentWorkshop.h"

#include <cmath>
#include <cstddef>
#include <string>

TEST_CASE("factory workshop provides distinct acoustic and electric instruments") {
    const auto presets = vll::audio::InstrumentWorkshop::factoryPresets();
    REQUIRE_EQ(presets.size(), std::size_t{7});
    for (const auto& preset : presets) REQUIRE(vll::audio::InstrumentWorkshop::validate(preset));
    REQUIRE(vll::audio::InstrumentWorkshop::factoryPreset("upright-warm").has_value());
    REQUIRE(vll::audio::InstrumentWorkshop::factoryPreset("felt-intimate").has_value());
    REQUIRE(vll::audio::InstrumentWorkshop::factoryPreset("rhodes-suitcase").has_value());
    REQUIRE(vll::audio::InstrumentWorkshop::factoryPreset("wurlitzer-200a").has_value());
}

TEST_CASE("electric piano presets expose complete ordered effect chains") {
    const auto rhodes = vll::audio::InstrumentWorkshop::factoryPreset("rhodes-suitcase");
    REQUIRE_EQ(rhodes->fxChain.size(), std::size_t{6});
    REQUIRE_EQ(rhodes->fxChain.front(), vll::audio::FxStage::Drive);
    REQUIRE_EQ(rhodes->fxChain.back(), vll::audio::FxStage::Reverb);
    REQUIRE(rhodes->tremoloDepth > 0.0F);
    REQUIRE(rhodes->equalizer.size() >= 2);
}

TEST_CASE("global and per-note tuning produce exact rendering ratios") {
    auto rhodes = *vll::audio::InstrumentWorkshop::factoryPreset("rhodes-suitcase");
    rhodes.globalFineCents = 5.0F;
    rhodes.perNoteCents[60] = -5.0F;
    REQUIRE(std::abs(vll::audio::InstrumentWorkshop::tuningRatio(rhodes, 60) - 1.0) < 1.0e-9);
    rhodes.perNoteCents[61] = 7.0F;
    REQUIRE(vll::audio::InstrumentWorkshop::tuningRatio(rhodes, 61) > 1.0);
    REQUIRE_EQ(vll::audio::InstrumentWorkshop::tuningRatio(rhodes, 128), 1.0);
}

TEST_CASE("parametric equalizer responds at active band centers") {
    auto preset = *vll::audio::InstrumentWorkshop::factoryPreset("studio-grand-dry");
    preset.equalizer = {{1000.0F, 6.0F, 2.0F, false}};
    const auto center = vll::audio::InstrumentWorkshop::equalizerGain(preset, 1000.0F);
    REQUIRE(std::abs(center - 1.995262F) < 0.001F);
    preset.equalizer.front().bypassed = true;
    REQUIRE_EQ(vll::audio::InstrumentWorkshop::equalizerGain(preset, 1000.0F), 1.0F);
}

TEST_CASE("workshop rejects unsafe parameter ranges") {
    auto preset = *vll::audio::InstrumentWorkshop::factoryPreset("wurlitzer-200a");
    preset.equalizer.front().q = 0.0F;
    REQUIRE(!vll::audio::InstrumentWorkshop::validate(preset));
    preset = *vll::audio::InstrumentWorkshop::factoryPreset("wurlitzer-200a");
    preset.perNoteCents[48] = 101.0F;
    REQUIRE(!vll::audio::InstrumentWorkshop::validate(preset));
}
