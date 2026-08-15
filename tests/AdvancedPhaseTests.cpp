#include "TestFramework.h"
#include "vll/freeplay/SessionAnalyzer.h"
#include "vll/practice/ProgressionPractice.h"
#include "vll/tutor/StructuredTutor.h"
TEST_CASE("free play analysis reports recurring motion habits") {std::vector<vll::Sonority>s{{{{48},{52}},0,1},{{{48},{53}},2,3},{{{50},{55}},4,5}};auto r=vll::freeplay::SessionAnalyzer{}.analyze(s,2);REQUIRE(r.complete);REQUIRE_EQ(r.paths.size(),std::size_t{2});REQUIRE_EQ(r.habits.size(),std::size_t{3});}
TEST_CASE("progression practice validates complete choruses") {vll::practice::PracticePlan p{"VL-17-A",{{"Cmaj7",{0,4,7,11}},{"Fmaj7",{5,9,0,4}}},2,2,false};std::vector<vll::Sonority>s{{{{48},{52}},0,1},{{{53},{57}},2,3},{{{48},{55}},4,5},{{{53},{60}},6,7}};auto r=vll::practice::ProgressionPractice{}.evaluate(p,s);REQUIRE(r.accepted);REQUIRE_EQ(r.completedChoruses,2);}
TEST_CASE("progression practice rejects exact non chord tones") {vll::practice::PracticePlan p{"VL-17-A",{{"C",{0,4,7}}},1,2,false};auto r=vll::practice::ProgressionPractice{}.evaluate(p,{{{{48},{49}},0,1}});REQUIRE(!r.accepted);REQUIRE_EQ(r.observations.front().code,std::string{"non_chord_tone"});}
TEST_CASE("optional tutor serializes only structured deterministic facts") {vll::tutor::TutorPayload p{"VL-05.2","VL-05.2-NV",{{"common_tone","F3 remained common."}}};vll::tutor::StructuredTutor t;REQUIRE(t.validate(p));REQUIRE(t.offlineExplanation(p).find("F3 remained common")!=std::string::npos);auto j=t.jsonPayload(p);REQUIRE(j.find("\"conceptId\":\"VL-05.2\"")!=std::string::npos);REQUIRE(j.find("common_tone")!=std::string::npos);}
TEST_CASE("tutor refuses payloads without deterministic facts") {vll::tutor::StructuredTutor t;vll::tutor::TutorPayload p;REQUIRE(!t.validate(p));REQUIRE_EQ(t.jsonPayload(p),std::string{"{}"});}
