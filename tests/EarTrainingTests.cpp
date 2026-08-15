#include "TestFramework.h"
#include "vll/ear/EarTrainingEngine.h"
#include <cstddef>
TEST_CASE("ear prompts reproduce exactly from seed") {vll::ear::EarTrainingEngine e;auto a=e.generate(42,vll::ear::Task::Direction),b=e.generate(42,vll::ear::Task::Direction);REQUIRE_EQ(a.progression[0].pitches,b.progression[0].pitches);REQUIRE_EQ(a.correctAnswer,b.correctAnswer);}
TEST_CASE("moving voice answers receive exact facts") {vll::ear::EarTrainingEngine e;auto p=e.generate(7,vll::ear::Task::Direction);REQUIRE(e.submit(p,-1).correct);auto wrong=e.submit(p,1);REQUIRE(!wrong.correct);REQUIRE(wrong.fact.fact.find("received 1")!=std::string::npos);}
TEST_CASE("common tone task counts retained voices") {vll::ear::EarTrainingEngine e;auto p=e.generate(9,vll::ear::Task::CommonTone);REQUIRE_EQ(p.correctAnswer,2);REQUIRE(e.submit(p,2).correct);}
TEST_CASE("isolated playback emits balanced note events") {vll::ear::EarTrainingEngine e;auto p=e.generate(9,vll::ear::Task::IsolatedVoice);auto events=e.isolatedPlayback(p);REQUIRE_EQ(events.size(),std::size_t{4});REQUIRE_EQ(events.front().type,vll::NoteEventType::NoteOn);REQUIRE_EQ(events.back().type,vll::NoteEventType::NoteOff);}
