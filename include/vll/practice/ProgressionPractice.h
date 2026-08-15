#pragma once
#include "vll/core/Types.h"
#include <string>
#include <vector>
namespace vll::practice { struct ProgressionEvent{std::string symbol;std::vector<int> pitchClasses;};struct PracticePlan{std::string id;std::vector<ProgressionEvent> progression;int choruses{1};int voiceCount{4};bool fixedSoprano{false};};struct PracticeEvaluation{bool accepted{false};int completedChoruses{0};std::vector<Observation> observations;};class ProgressionPractice final{public:[[nodiscard]] bool validate(const PracticePlan&)const;[[nodiscard]] PracticeEvaluation evaluate(const PracticePlan&,const std::vector<Sonority>&)const;};}
