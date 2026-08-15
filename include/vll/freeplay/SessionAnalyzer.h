#pragma once
#include "vll/core/Types.h"
#include <string>
#include <vector>
namespace vll::freeplay { struct Habit{std::string code;int count{0};}; struct SessionAnalysis{bool complete{false};std::vector<VoicePath> paths;std::vector<Habit> habits;std::string error;}; class SessionAnalyzer final{public:[[nodiscard]] SessionAnalysis analyze(const std::vector<Sonority>&,int voices)const;}; }
