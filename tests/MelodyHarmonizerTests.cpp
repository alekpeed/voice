#include "TestFramework.h"
#include "vll/harmony/MelodyHarmonizer.h"

#include <cstddef>

namespace {

vll::harmony::HarmonizationRequest request() {
    return {{{{60}, "Dm7", {2, 5, 9, 0}},
             {{59}, "G7", {7, 11, 2, 5}},
             {{59}, "Cmaj7", {0, 4, 7, 11}}},
            4, 36, 84, vll::Sonority{{{50}, {53}, {57}, {60}}, 0, 0}, 3};
}

} // namespace

TEST_CASE("melody harmonizer preserves the prescribed soprano") {
    const auto result = vll::harmony::MelodyHarmonizer{}.harmonize(request());
    REQUIRE(result.complete);
    REQUIRE_EQ(result.events.size(), std::size_t{3});
    REQUIRE_EQ(result.selectedVoicePaths.size(), std::size_t{4});
    for (std::size_t index = 0; index < result.events.size(); ++index) {
        REQUIRE(!result.events[index].candidates.empty());
        REQUIRE_EQ(result.events[index].candidates.front().voicing.pitches.back(),
                   request().events[index].melody);
    }
}

TEST_CASE("melody candidates are ranked by smoothness") {
    const auto result = vll::harmony::MelodyHarmonizer{}.harmonize(request());
    for (const auto& event : result.events) {
        for (std::size_t index = 1; index < event.candidates.size(); ++index) {
            REQUIRE(event.candidates[index - 1].totalDisplacement <=
                    event.candidates[index].totalDisplacement);
        }
    }
}

TEST_CASE("melody harmonizer rejects pitches outside supplied harmony") {
    auto invalid = request();
    invalid.events[1].melody = {58};
    const auto result = vll::harmony::MelodyHarmonizer{}.harmonize(invalid);
    REQUIRE(!result.complete);
    REQUIRE_EQ(result.error, std::string{"Melody pitch is not present in the supplied harmony."});
}

TEST_CASE("melody harmonizer rejects invalid dimensions") {
    auto invalid = request();
    invalid.voiceCount = 5;
    REQUIRE(!vll::harmony::MelodyHarmonizer{}.harmonize(invalid).complete);
    invalid = request();
    invalid.initialVoicing->pitches.pop_back();
    REQUIRE_EQ(vll::harmony::MelodyHarmonizer{}.harmonize(invalid).error,
               std::string{"Initial voicing does not match the requested voice count."});
}
