#include "TestFramework.h"
#include "vll/barry/BarryHarrisEngine.h"
#include <cstddef>
TEST_CASE("major and minor sixth diminished fields are exact") {vll::barry::BarryHarrisEngine e;auto m=e.field(0,false);REQUIRE_EQ(m.sixth,std::vector<int>({0,4,7,9}));REQUIRE_EQ(m.diminished,std::vector<int>({2,5,8,11}));auto n=e.field(0,true);REQUIRE_EQ(n.sixth,std::vector<int>({0,3,7,9}));}
TEST_CASE("melody harmonization selects its alternating family") {vll::barry::BarryHarrisEngine e;auto f=e.field(0,false);auto stable=e.inversions(f,{72},4);auto passing=e.inversions(f,{74},4);REQUIRE(!stable.empty());REQUIRE(!passing.empty());REQUIRE_EQ(stable.front().pitches.back(),vll::Pitch{72});REQUIRE_EQ(e.classify(stable.front(),f),vll::barry::SixthFamily::Major);REQUIRE_EQ(e.classify(passing.front(),f),vll::barry::SixthFamily::Diminished);}
TEST_CASE("borrowing identifies moved voices") {vll::barry::BarryHarrisEngine e;auto f=e.field(0,false);vll::Sonority pure{{{48},{52},{55},{57}},0,0};auto b=e.borrow(pure,f,2);REQUIRE_EQ(b.borrowedVoices.size(),std::size_t{2});REQUIRE(b.borrowed.pitches!=pure.pitches);}
TEST_CASE("diminished field exposes four related dominants") {vll::barry::BarryHarrisEngine e;REQUIRE_EQ(e.relatedDominantRoots(e.field(0,false)),std::vector<int>({1,4,7,10}));}
TEST_CASE("minor sixth can be reinterpreted over independent dominant bass") {vll::barry::BarryHarrisEngine e;auto s=e.reinterpretMinorSixOverBass(2,{43});REQUIRE_EQ(s.pitches.front(),vll::Pitch{43});REQUIRE_EQ(s.pitches.size(),std::size_t{5});}
