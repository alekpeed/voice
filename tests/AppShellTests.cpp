#include "TestFramework.h"
#include "vll/application/AppShell.h"

TEST_CASE("app shell exposes all product areas") {
    vll::application::AppShell shell;
    REQUIRE_EQ(shell.routes().size(), static_cast<std::size_t>(10));
    REQUIRE_EQ(shell.activeRoute(), vll::application::Route::Home);
}

TEST_CASE("instrument route exposes workshop panel") {
    vll::application::AppShell shell;
    shell.navigate(vll::application::Route::InstrumentsSettings);
    REQUIRE(shell.layout().rightPanel.find("workshop") != std::string::npos);
}

TEST_CASE("visualizer route exposes Phase 4 controls") {
    vll::application::AppShell shell;
    shell.navigate(vll::application::Route::Visualizer);
    const auto layout = shell.layout();
    REQUIRE(layout.centerPanel.find("Voice graph") != std::string::npos);
    REQUIRE(layout.centerPanel.find("timeline") != std::string::npos);
    REQUIRE(layout.rightPanel.find("isolation") != std::string::npos);
    REQUIRE(layout.rightPanel.find("zoom") != std::string::npos);
}

TEST_CASE("study routes expose Phase 5 notation controls") {
    vll::application::AppShell shell;
    shell.navigate(vll::application::Route::Practice);
    const auto layout = shell.layout();
    REQUIRE(layout.centerPanel.find("grand staff") != std::string::npos);
    REQUIRE(layout.rightPanel.find("scale") != std::string::npos);
    REQUIRE(layout.rightPanel.find("voice highlights") != std::string::npos);
    REQUIRE(layout.rightPanel.find("fingering") != std::string::npos);
}
