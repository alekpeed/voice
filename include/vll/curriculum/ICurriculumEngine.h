#pragma once

#include <optional>
#include <string>
#include <vector>

namespace vll::curriculum {

enum class Competency { NotStarted, Introduced, Developing, Reliable, Fluent };

struct Concept {
    std::string id;
    std::string title;
    std::vector<std::string> prerequisites;
    std::optional<std::string> bookReference;
};

class ICurriculumEngine {
public:
    virtual ~ICurriculumEngine() = default;
    [[nodiscard]] virtual std::optional<Concept> conceptById(const std::string& id) const = 0;
    [[nodiscard]] virtual Competency competency(const std::string& conceptId) const = 0;
    virtual void recordEvidence(const std::string& conceptId, bool successful,
                                bool usedHints, bool transposed) = 0;
};

} // namespace vll::curriculum
