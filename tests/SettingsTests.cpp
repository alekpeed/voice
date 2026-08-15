#include "TestFramework.h"
#include "vll/core/Settings.h"

#include <filesystem>

TEST_CASE("settings reject invalid tuning") {
    auto settings = vll::Settings::defaults();
    REQUIRE(!settings.set("audio.a4_hz", "900"));
    REQUIRE(settings.set("audio.a4_hz", "442.0"));
    REQUIRE_EQ(settings.value("audio.a4_hz"), std::string("442.0"));
}

TEST_CASE("settings round trip") {
    const auto path = std::filesystem::temp_directory_path() / "vll-settings-test.conf";
    auto settings = vll::Settings::defaults();
    REQUIRE(settings.set("midi.chord_window_ms", "95"));
    REQUIRE(settings.save(path));
    const auto loaded = vll::Settings::load(path);
    REQUIRE(loaded.has_value());
    REQUIRE_EQ(loaded->value("midi.chord_window_ms"), std::string("95"));
    std::filesystem::remove(path);
}
