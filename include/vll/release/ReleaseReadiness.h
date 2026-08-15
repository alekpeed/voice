#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace vll::release {
struct AccessibilityPreferences{bool highContrast{false};bool reducedMotion{false};bool screenReaderLabels{true};float interfaceScale{1.0F};};
struct ProgressEntry{std::string conceptId;int competency{0};std::uint64_t evidenceCount{0};};
struct Backup{int schemaVersion{1};std::vector<ProgressEntry> progress;AccessibilityPreferences accessibility;};
struct ImportResult{std::optional<Backup> backup;std::string error;};
class ReleaseReadiness final{public:[[nodiscard]] static bool validateAccessibility(const AccessibilityPreferences&);[[nodiscard]] static std::vector<std::string> onboardingSteps();[[nodiscard]] static std::string exportBackup(const Backup&);[[nodiscard]] static ImportResult importBackup(const std::string&);};
}
