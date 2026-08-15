#include "TestFramework.h"
#include "vll/curriculum/BookCurriculumEngine.h"
#include "vll/curriculum/CurriculumCatalog.h"

#include <cstddef>
#include <string>

using vll::curriculum::BookCurriculumEngine;
using vll::curriculum::Competency;

TEST_CASE("all canonical voice-leading IDs resolve to book-linked concepts") {
    const BookCurriculumEngine engine;
    for (const auto& reference : vll::curriculum::CurriculumCatalog::voiceLeadingConcepts()) {
        const auto item = engine.conceptById(std::string{reference.id});
        REQUIRE(item.has_value());
        REQUIRE_EQ(item->id, std::string{reference.id});
        REQUIRE(item->bookReference.has_value());
        REQUIRE(item->bookReference->find("VL-STUDY:p") == 0);
        REQUIRE(item->bookReference->find(";VL-PRACTICE:p") != std::string::npos);
    }
    REQUIRE(!engine.conceptById("VL-99.9").has_value());
}

TEST_CASE("book routes preserve pages unit and five printed exercises") {
    const BookCurriculumEngine engine;
    const auto route = engine.bookRoute("VL-05.2");
    REQUIRE(route.has_value());
    REQUIRE_EQ(route->studySourceId, std::string{"VL-STUDY"});
    REQUIRE_EQ(route->studyPage, 15);
    REQUIRE_EQ(route->practiceSourceId, std::string{"VL-PRACTICE"});
    REQUIRE_EQ(route->practicePage, 11);
    REQUIRE_EQ(route->practiceUnitId, std::string{"VL-U05"});
    REQUIRE_EQ(route->exerciseIds.size(), std::size_t{5});
    REQUIRE_EQ(route->exerciseIds.front(), std::string{"VL-EX-05A"});
    REQUIRE_EQ(route->exerciseIds.back(), std::string{"VL-EX-05E"});
}

TEST_CASE("prerequisite graph follows stable concept order") {
    BookCurriculumEngine engine;
    REQUIRE(engine.prerequisitesMet("VL-01.1"));
    const auto second = engine.conceptById("VL-01.2");
    REQUIRE_EQ(second->prerequisites.size(), std::size_t{1});
    REQUIRE_EQ(second->prerequisites.front(), std::string{"VL-01.1"});
    REQUIRE(!engine.prerequisitesMet("VL-01.2"));
    REQUIRE_EQ(engine.unmetPrerequisites("VL-01.2").front(), std::string{"VL-01.1"});

    engine.recordEvidence("VL-01.1", true, false, false);
    engine.recordEvidence("VL-01.1", true, false, false);
    engine.recordEvidence("VL-01.1", true, false, false);
    REQUIRE(engine.prerequisitesMet("VL-01.2"));

    const auto chapterBoundary = engine.conceptById("VL-02.1");
    REQUIRE_EQ(chapterBoundary->prerequisites.front(), std::string{"VL-01.4"});
}

TEST_CASE("competency advances only from exact evidence thresholds") {
    BookCurriculumEngine engine;
    const std::string id{"VL-05.2"};
    REQUIRE_EQ(engine.competency(id), Competency::NotStarted);
    engine.recordEvidence(id, false, false, false);
    REQUIRE_EQ(engine.competency(id), Competency::Introduced);
    engine.recordEvidence(id, true, true, false);
    engine.recordEvidence(id, true, true, false);
    REQUIRE_EQ(engine.competency(id), Competency::Developing);
    engine.recordEvidence(id, true, false, false);
    engine.recordEvidence(id, true, false, true);
    engine.recordEvidence(id, true, false, true);
    REQUIRE_EQ(engine.competency(id), Competency::Reliable);
    engine.recordEvidence(id, true, false, false);
    REQUIRE_EQ(engine.competency(id), Competency::Fluent);

    const auto evidence = engine.evidence(id);
    REQUIRE_EQ(evidence.attempts, std::size_t{7});
    REQUIRE_EQ(evidence.successes, std::size_t{6});
    REQUIRE_EQ(evidence.unassistedSuccesses, std::size_t{4});
    REQUIRE_EQ(evidence.transposedUnassistedSuccesses, std::size_t{2});
}

TEST_CASE("invalid IDs do not create competency evidence") {
    BookCurriculumEngine engine;
    engine.recordEvidence("VL-99.9", true, false, true);
    REQUIRE_EQ(engine.evidence("VL-99.9").attempts, std::size_t{0});
    REQUIRE_EQ(engine.competency("VL-99.9"), Competency::NotStarted);
    REQUIRE(!engine.prerequisitesMet("VL-99.9"));
}

TEST_CASE("competency names match the documented book mapping") {
    REQUIRE_EQ(vll::curriculum::competencyName(Competency::NotStarted),
               std::string{"Not Started"});
    REQUIRE_EQ(vll::curriculum::competencyName(Competency::Introduced),
               std::string{"Introduced"});
    REQUIRE_EQ(vll::curriculum::competencyName(Competency::Developing),
               std::string{"Developing"});
    REQUIRE_EQ(vll::curriculum::competencyName(Competency::Reliable),
               std::string{"Reliable"});
    REQUIRE_EQ(vll::curriculum::competencyName(Competency::Fluent),
               std::string{"Fluent"});
}
