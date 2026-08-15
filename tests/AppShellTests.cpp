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
