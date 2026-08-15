#pragma once
#include "vll/core/Types.h"
#include <string>
#include <vector>
namespace vll::barry {
enum class SixthFamily { Major, Minor, Diminished };
struct BarryField { int key{0}; bool minor{false}; std::vector<int> sixth; std::vector<int> diminished; std::vector<int> collection; };
struct BorrowedVoicing { Sonority pure; Sonority borrowed; std::vector<VoiceId> borrowedVoices; };
class BarryHarrisEngine final {
public:
 [[nodiscard]] BarryField field(int key, bool minor) const;
 [[nodiscard]] SixthFamily classify(const Sonority& voicing, const BarryField& field) const;
 [[nodiscard]] std::vector<Sonority> inversions(const BarryField& field, Pitch melody, int voices=4) const;
 [[nodiscard]] BorrowedVoicing borrow(const Sonority& pure, const BarryField& field, int count) const;
 [[nodiscard]] std::vector<int> relatedDominantRoots(const BarryField& field) const;
 [[nodiscard]] Sonority reinterpretMinorSixOverBass(int minorRoot, Pitch bass) const;
};
}
