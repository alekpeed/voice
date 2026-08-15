#include "vll/application/AppShell.h"

#include <sstream>

namespace vll::application {

AppShell::AppShell()
    : routes_{Route::Home, Route::Learn, Route::Practice, Route::Visualizer,
              Route::Lab, Route::BarryHarris, Route::EarTraining, Route::FreePlay,
              Route::Progress, Route::InstrumentsSettings} {}

const std::vector<Route>& AppShell::routes() const noexcept { return routes_; }
Route AppShell::activeRoute() const noexcept { return activeRoute_; }
void AppShell::navigate(const Route route) noexcept { activeRoute_ = route; }

WorkspaceLayout AppShell::layout() const {
    if (activeRoute_ == Route::Visualizer) {
        return {
            "Navigation / captured progressions",
            "Voice graph / event timeline / relevant keyboard",
            "Voice isolation / playback / zoom / instrument controls"
        };
    }
    if (activeRoute_ == Route::Learn || activeRoute_ == Route::Practice ||
        activeRoute_ == Route::BarryHarris) {
        return {
            "Navigation / lesson or exercise list",
            "Compact grand staff / voice graph / relevant keyboard",
            "Notation scale / chord symbols / voice highlights / fingering"
        };
    }
    return {
        activeRoute_ == Route::Home ? "Navigation" : "Navigation / lesson or exercise list",
        activeRoute_ == Route::Home ? "Connect MIDI keyboard to begin" :
                                     "Compact notation / voice graph / relevant keyboard",
        activeRoute_ == Route::InstrumentsSettings ? "Instrument quick controls / workshop" :
                                                     "Analysis / constraints / instrument controls"
    };
}

std::string AppShell::renderTextSnapshot() const {
    const auto panels = layout();
    std::ostringstream output;
    output << "Voice Leading Lab | " << routeName(activeRoute_) << '\n'
           << "Left: " << panels.leftPanel << '\n'
           << "Center: " << panels.centerPanel << '\n'
           << "Right: " << panels.rightPanel << '\n';
    return output.str();
}

std::string AppShell::routeName(const Route route) {
    switch (route) {
        case Route::Home: return "Home";
        case Route::Learn: return "Learn";
        case Route::Practice: return "Practice";
        case Route::Visualizer: return "Visualizer";
        case Route::Lab: return "Lab";
        case Route::BarryHarris: return "Barry Harris";
        case Route::EarTraining: return "Ear Training";
        case Route::FreePlay: return "Free Play";
        case Route::Progress: return "Progress";
        case Route::InstrumentsSettings: return "Instruments / Settings";
    }
    return "Unknown";
}

} // namespace vll::application
