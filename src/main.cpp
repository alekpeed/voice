#include "vll/application/AppShell.h"
#include "vll/core/Logger.h"
#include "vll/core/Settings.h"

#include <iostream>
#include <string_view>

int main(const int argc, char** argv) {
    const bool smokeTest = argc > 1 && std::string_view(argv[1]) == "--smoke-test";
    vll::Logger logger;
    logger.write(vll::LogLevel::Info, "application", "starting Voice Leading Lab Phase 0 shell");

    const auto settings = vll::Settings::defaults();
    vll::application::AppShell shell;
    std::cout << shell.renderTextSnapshot();
    std::cout << "A4 reference: " << settings.value("audio.a4_hz") << " Hz\n";

    if (smokeTest) std::cout << "SMOKE_TEST_OK\n";
    return 0;
}
