#include "TestFramework.h"
#include "vll/lab/ReharmonizationLab.h"

#include <cstddef>
#include <optional>
#include <string>

namespace {

vll::lab::LabRequest labRequest() {
    return {"VL-13.4-LAB-A",
            {{"Dm7", {2, 5, 9, 0}}, {"G7", {7, 11, 2, 5}}, {"Cmaj7", {0, 4, 7, 11}}},
            {{{50}, {53}, {57}, {60}}, 0, 0}, 4,
            {vll::Pitch{60}, vll::Pitch{59}, vll::Pitch{59}},
            {vll::Pitch{50}, vll::Pitch{43}, vll::Pitch{48}}, {}};
}

} // namespace

TEST_CASE("lab realizes independent fixed soprano and bass lines") {
    const auto request = labRequest();
    const auto result = vll::lab::ReharmonizationLab{}.realize(request);
    REQUIRE(result.complete);
    REQUIRE_EQ(result.voicings.size(), std::size_t{3});
    REQUIRE_EQ(result.voicePaths.size(), std::size_t{4});
    for (std::size_t index = 0; index < result.voicings.size(); ++index) {
        REQUIRE_EQ(result.voicings[index].pitches.front(), *request.bassLine[index]);
        REQUIRE_EQ(result.voicings[index].pitches.back(), *request.sopranoLine[index]);
    }
}

TEST_CASE("lab enforces user-defined inner lines") {
    auto request = labRequest();
    request.requiredInnerLines = {{{53}, {53}, {52}}};
    const auto result = vll::lab::ReharmonizationLab{}.realize(request);
    REQUIRE(result.complete);
    REQUIRE_EQ(result.voicings[1].pitches[1], vll::Pitch{53});
    REQUIRE_EQ(result.voicings[2].pitches[1], vll::Pitch{52});
}

TEST_CASE("lab reports impossible line constraints") {
    auto request = labRequest();
    request.sopranoLine[1] = vll::Pitch{58};
    const auto result = vll::lab::ReharmonizationLab{}.realize(request);
    REQUIRE(!result.complete);
    REQUIRE(result.error.find("event 2") != std::string::npos);
}

TEST_CASE("lab saves and lists named definitions deterministically") {
    vll::lab::ReharmonizationLab lab;
    REQUIRE(lab.save("B study", labRequest()));
    REQUIRE(lab.save("A study", labRequest()));
    REQUIRE(!lab.save("", labRequest()));
    REQUIRE_EQ(lab.savedNames(), std::vector<std::string>({"A study", "B study"}));
    REQUIRE(lab.load("A study").has_value());
    REQUIRE(!lab.load("missing").has_value());
}
