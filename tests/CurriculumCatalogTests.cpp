#include "TestFramework.h"
#include "vll/curriculum/CurriculumCatalog.h"

#include <array>
#include <cstddef>
#include <string_view>
#include <unordered_set>

namespace {

template <typename Range, typename IdFunction>
bool hasUniqueIds(const Range& range, IdFunction idFunction) {
    std::unordered_set<std::string_view> ids;
    for (const auto& item : range) {
        if (!ids.insert(idFunction(item)).second) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("PDF curriculum catalog has the complete source set") {
    const auto sources = vll::curriculum::CurriculumCatalog::sourceDocuments();
    REQUIRE_EQ(sources.size(), std::size_t{3});
    REQUIRE(hasUniqueIds(sources, [](const auto& item) { return item.id; }));
    for (const auto& source : sources) {
        REQUIRE_EQ(source.sha256.size(), std::size_t{64});
        REQUIRE(source.pageCount > 0);
    }
}

TEST_CASE("voice-leading concepts map one-to-one across both volumes") {
    const auto concepts = vll::curriculum::CurriculumCatalog::voiceLeadingConcepts();
    const auto units = vll::curriculum::CurriculumCatalog::practiceUnits();
    REQUIRE_EQ(concepts.size(), std::size_t{64});
    REQUIRE_EQ(units.size(), std::size_t{16});
    REQUIRE(hasUniqueIds(concepts, [](const auto& item) { return item.id; }));
    REQUIRE(hasUniqueIds(units, [](const auto& item) { return item.id; }));

    std::unordered_set<std::string_view> referencedConceptIds;
    for (const auto& unit : units) {
        REQUIRE(unit.unit >= 1 && unit.unit <= 16);
        for (const auto conceptId : unit.conceptIds) {
            referencedConceptIds.insert(conceptId);
        }
    }
    REQUIRE_EQ(referencedConceptIds.size(), concepts.size());
    for (const auto& conceptReference : concepts) {
        REQUIRE(referencedConceptIds.contains(conceptReference.id));
        REQUIRE_EQ(conceptReference.chapter, conceptReference.practiceUnit);
        REQUIRE(conceptReference.studyGuidePage > 0);
        REQUIRE(conceptReference.practiceCompanionPage > 0);
    }
}

TEST_CASE("practice companion contains five exercises per unit and twelve etudes") {
    const auto exercises = vll::curriculum::CurriculumCatalog::practiceExercises();
    const auto etudes = vll::curriculum::CurriculumCatalog::practiceEtudes();
    REQUIRE_EQ(exercises.size(), std::size_t{80});
    REQUIRE_EQ(etudes.size(), std::size_t{12});
    REQUIRE(hasUniqueIds(exercises, [](const auto& item) { return item.id; }));
    REQUIRE(hasUniqueIds(etudes, [](const auto& item) { return item.id; }));

    std::array<int, 16> exercisesPerUnit{};
    for (const auto& exercise : exercises) {
        REQUIRE(exercise.unit >= 1 && exercise.unit <= 16);
        ++exercisesPerUnit.at(static_cast<std::size_t>(exercise.unit - 1));
    }
    for (const auto count : exercisesPerUnit) {
        REQUIRE_EQ(count, 5);
    }
}

TEST_CASE("Barry Harris guide is fully indexed") {
    const auto chapters = vll::curriculum::CurriculumCatalog::barryChapters();
    const auto appendices = vll::curriculum::CurriculumCatalog::barryAppendices();
    REQUIRE_EQ(chapters.size(), std::size_t{33});
    REQUIRE_EQ(appendices.size(), std::size_t{5});
    REQUIRE(hasUniqueIds(chapters, [](const auto& item) { return item.id; }));
    REQUIRE(hasUniqueIds(appendices, [](const auto& item) { return item.id; }));

    for (std::size_t index = 0; index < chapters.size(); ++index) {
        const auto& chapter = chapters[index];
        const int expectedPart = index < 7 ? 1 : index < 14 ? 2 : index < 21 ? 3 : index < 28 ? 4 : 5;
        REQUIRE_EQ(chapter.part, expectedPart);
        REQUIRE(chapter.startPage > 0);
    }
}

TEST_CASE("book benchmark levels map monotonically to app competency") {
    const auto mapping = vll::curriculum::CurriculumCatalog::benchmarkMapping();
    REQUIRE_EQ(mapping.size(), std::size_t{5});
    for (std::size_t index = 0; index < mapping.size(); ++index) {
        REQUIRE_EQ(mapping[index].bookLevel, static_cast<int>(index));
        REQUIRE(!mapping[index].bookLabel.empty());
        REQUIRE(!mapping[index].appCompetency.empty());
    }
}
