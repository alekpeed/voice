#pragma once

#include <string>
#include <vector>

namespace vll::application {

enum class Route {
    Home, Learn, Practice, Visualizer, Lab, BarryHarris,
    EarTraining, FreePlay, Progress, InstrumentsSettings
};

struct WorkspaceLayout {
    std::string leftPanel;
    std::string centerPanel;
    std::string rightPanel;
};

class AppShell {
public:
    AppShell();
    [[nodiscard]] const std::vector<Route>& routes() const noexcept;
    [[nodiscard]] Route activeRoute() const noexcept;
    void navigate(Route route) noexcept;
    [[nodiscard]] WorkspaceLayout layout() const;
    [[nodiscard]] std::string renderTextSnapshot() const;
    [[nodiscard]] static std::string routeName(Route route);

private:
    std::vector<Route> routes_;
    Route activeRoute_{Route::Home};
};

} // namespace vll::application
