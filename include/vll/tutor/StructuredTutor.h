#pragma once
#include "vll/core/Types.h"
#include <string>
#include <vector>
namespace vll::tutor {struct TutorPayload{std::string conceptId;std::string exerciseId;std::vector<Observation> facts;};class StructuredTutor final{public:[[nodiscard]] bool validate(const TutorPayload&)const;[[nodiscard]] std::string offlineExplanation(const TutorPayload&)const;[[nodiscard]] std::string jsonPayload(const TutorPayload&)const;};}
