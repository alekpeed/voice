#include "TestFramework.h"
#include "vll/core/Types.h"

TEST_CASE("pitch class normalizes negative MIDI values") {
    REQUIRE_EQ(vll::Pitch{-1}.pitchClass(), 11);
    REQUIRE_EQ(vll::Pitch{60}.pitchClass(), 0);
}

TEST_CASE("movement sizes use voice-leading definitions") {
    REQUIRE_EQ(vll::classifyMovement(0), vll::MovementSize::Stationary);
    REQUIRE_EQ(vll::classifyMovement(-1), vll::MovementSize::Semitone);
    REQUIRE_EQ(vll::classifyMovement(2), vll::MovementSize::WholeStep);
    REQUIRE_EQ(vll::classifyMovement(-5), vll::MovementSize::Leap);
}
