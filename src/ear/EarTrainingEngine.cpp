#include "vll/ear/EarTrainingEngine.h"
#include <algorithm>
namespace vll::ear {
EarPrompt EarTrainingEngine::generate(std::uint64_t seed,Task task) const {const int key=static_cast<int>((seed^(seed>>32))%12);EarPrompt p{"VL-02.2-EAR",seed,task,{{{{48+key},{52+key},{55+key}},0,100000},{{{48+key},{51+key},{55+key}},1000000,1100000}},2,0};if(task==Task::Direction)p.correctAnswer=-1;else if(task==Task::CommonTone)p.correctAnswer=2;else if(task==Task::IsolatedVoice)p.correctAnswer=51+key;else p.correctAnswer=1;return p;}
EarResult EarTrainingEngine::submit(const EarPrompt& p,int answer) const {const bool ok=answer==p.correctAnswer;return {ok,{ok?"ear_answer_correct":"ear_answer_incorrect","Expected answer "+std::to_string(p.correctAnswer)+"; received "+std::to_string(answer)+"."}};}
std::vector<NoteEvent> EarTrainingEngine::isolatedPlayback(const EarPrompt& p) const {std::vector<NoteEvent> e;const auto i=static_cast<std::size_t>(p.targetVoice-1);for(const auto&s:p.progression)if(i<s.pitches.size()){e.push_back({NoteEventType::NoteOn,s.pitches[i],0.7F,s.startedAt,1});e.push_back({NoteEventType::NoteOff,s.pitches[i],0.0F,s.endedAt,1});}return e;}
}
