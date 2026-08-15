#pragma once

#include <array>
#include <span>
#include <string_view>

namespace vll::curriculum {

struct SourceDocumentReference {
    std::string_view id;
    std::string_view title;
    std::string_view archivePath;
    std::string_view sha256;
    int pageCount;
};

struct VoiceLeadingConceptReference {
    std::string_view id;
    std::string_view title;
    int chapter;
    int studyGuidePage;
    int practiceUnit;
    int practiceCompanionPage;
};

struct PracticeUnitReference {
    std::string_view id;
    std::string_view title;
    int unit;
    int startPage;
    std::array<std::string_view, 4> conceptIds;
};

struct PracticeExerciseReference {
    std::string_view id;
    std::string_view bookLabel;
    std::string_view title;
    int unit;
};

struct PracticeEtudeReference {
    std::string_view id;
    std::string_view title;
    int startPage;
};

struct BarryChapterReference {
    std::string_view id;
    std::string_view title;
    int part;
    int startPage;
};

struct BarryAppendixReference {
    std::string_view id;
    std::string_view title;
    int startPage;
};

struct BenchmarkReference {
    int bookLevel;
    std::string_view bookLabel;
    std::string_view appCompetency;
};

class CurriculumCatalog final {
public:
    [[nodiscard]] static std::span<const SourceDocumentReference> sourceDocuments();
    [[nodiscard]] static std::span<const VoiceLeadingConceptReference> voiceLeadingConcepts();
    [[nodiscard]] static std::span<const PracticeUnitReference> practiceUnits();
    [[nodiscard]] static std::span<const PracticeExerciseReference> practiceExercises();
    [[nodiscard]] static std::span<const PracticeEtudeReference> practiceEtudes();
    [[nodiscard]] static std::span<const BarryChapterReference> barryChapters();
    [[nodiscard]] static std::span<const BarryAppendixReference> barryAppendices();
    [[nodiscard]] static std::span<const BenchmarkReference> benchmarkMapping();
};

} // namespace vll::curriculum
