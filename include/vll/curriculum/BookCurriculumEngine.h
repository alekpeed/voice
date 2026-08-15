#pragma once

#include "vll/curriculum/ICurriculumEngine.h"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace vll::curriculum {

struct BookRoute {
    std::string conceptId;
    std::string studySourceId;
    int studyPage{0};
    std::string practiceSourceId;
    int practicePage{0};
    std::string practiceUnitId;
    std::vector<std::string> exerciseIds;
};

struct CompetencyEvidence {
    std::size_t attempts{0};
    std::size_t successes{0};
    std::size_t unassistedSuccesses{0};
    std::size_t transposedUnassistedSuccesses{0};
};

class BookCurriculumEngine final : public ICurriculumEngine {
public:
    [[nodiscard]] std::optional<Concept> conceptById(const std::string& id) const override;
    [[nodiscard]] Competency competency(const std::string& conceptId) const override;
    void recordEvidence(const std::string& conceptId, bool successful,
                        bool usedHints, bool transposed) override;

    [[nodiscard]] std::optional<BookRoute> bookRoute(const std::string& conceptId) const;
    [[nodiscard]] CompetencyEvidence evidence(const std::string& conceptId) const;
    [[nodiscard]] bool prerequisitesMet(const std::string& conceptId) const;
    [[nodiscard]] std::vector<std::string> unmetPrerequisites(
        const std::string& conceptId) const;

private:
    std::unordered_map<std::string, CompetencyEvidence> evidence_;
};

[[nodiscard]] std::string competencyName(Competency competency);

} // namespace vll::curriculum
