#include "vll/curriculum/BookCurriculumEngine.h"

#include "vll/curriculum/CurriculumCatalog.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace vll::curriculum {
namespace {

std::optional<std::size_t> conceptIndex(const std::string& id) {
    const auto concepts = CurriculumCatalog::voiceLeadingConcepts();
    const auto found = std::ranges::find(concepts, id, &VoiceLeadingConceptReference::id);
    if (found == concepts.end()) return std::nullopt;
    return static_cast<std::size_t>(std::distance(concepts.begin(), found));
}

std::vector<std::string> prerequisitesFor(const std::size_t index) {
    if (index == 0) return {};
    const auto concepts = CurriculumCatalog::voiceLeadingConcepts();
    return {std::string{concepts[index - 1].id}};
}

std::string unitId(const int unit) {
    std::ostringstream id;
    id << "VL-U" << std::setw(2) << std::setfill('0') << unit;
    return id.str();
}

} // namespace

std::optional<Concept> BookCurriculumEngine::conceptById(const std::string& id) const {
    const auto index = conceptIndex(id);
    if (!index) return std::nullopt;
    const auto& reference = CurriculumCatalog::voiceLeadingConcepts()[*index];
    std::ostringstream bookReference;
    bookReference << "VL-STUDY:p" << reference.studyGuidePage
                  << ";VL-PRACTICE:p" << reference.practiceCompanionPage;
    return Concept{std::string{reference.id}, std::string{reference.title},
                   prerequisitesFor(*index), bookReference.str()};
}

Competency BookCurriculumEngine::competency(const std::string& conceptId) const {
    if (!conceptIndex(conceptId)) return Competency::NotStarted;
    const auto state = evidence(conceptId);
    if (state.transposedUnassistedSuccesses >= 2 && state.unassistedSuccesses >= 4) {
        return Competency::Fluent;
    }
    if (state.unassistedSuccesses >= 3) return Competency::Reliable;
    if (state.successes >= 2) return Competency::Developing;
    if (state.attempts > 0) return Competency::Introduced;
    return Competency::NotStarted;
}

void BookCurriculumEngine::recordEvidence(const std::string& conceptId,
                                          const bool successful,
                                          const bool usedHints,
                                          const bool transposed) {
    if (!conceptIndex(conceptId)) return;
    auto& state = evidence_[conceptId];
    ++state.attempts;
    if (!successful) return;
    ++state.successes;
    if (usedHints) return;
    ++state.unassistedSuccesses;
    if (transposed) ++state.transposedUnassistedSuccesses;
}

std::optional<BookRoute> BookCurriculumEngine::bookRoute(
    const std::string& conceptId) const {
    const auto index = conceptIndex(conceptId);
    if (!index) return std::nullopt;
    const auto& reference = CurriculumCatalog::voiceLeadingConcepts()[*index];
    BookRoute route{
        std::string{reference.id}, "VL-STUDY", reference.studyGuidePage,
        "VL-PRACTICE", reference.practiceCompanionPage,
        unitId(reference.practiceUnit), {}};
    for (const auto& exercise : CurriculumCatalog::practiceExercises()) {
        if (exercise.unit == reference.practiceUnit) {
            route.exerciseIds.emplace_back(exercise.id);
        }
    }
    return route;
}

CompetencyEvidence BookCurriculumEngine::evidence(const std::string& conceptId) const {
    const auto found = evidence_.find(conceptId);
    return found == evidence_.end() ? CompetencyEvidence{} : found->second;
}

bool BookCurriculumEngine::prerequisitesMet(const std::string& conceptId) const {
    return unmetPrerequisites(conceptId).empty();
}

std::vector<std::string> BookCurriculumEngine::unmetPrerequisites(
    const std::string& conceptId) const {
    const auto item = conceptById(conceptId);
    if (!item) return {conceptId};
    std::vector<std::string> unmet;
    for (const auto& prerequisite : item->prerequisites) {
        const auto level = competency(prerequisite);
        if (level != Competency::Reliable && level != Competency::Fluent) {
            unmet.push_back(prerequisite);
        }
    }
    return unmet;
}

std::string competencyName(const Competency competency) {
    switch (competency) {
        case Competency::NotStarted: return "Not Started";
        case Competency::Introduced: return "Introduced";
        case Competency::Developing: return "Developing";
        case Competency::Reliable: return "Reliable";
        case Competency::Fluent: return "Fluent";
    }
    return "Not Started";
}

} // namespace vll::curriculum
