#include "vll/freeplay/SessionAnalyzer.h"
#include "vll/analysis/VoiceAssigner.h"
#include "vll/visualization/VoicePathBuilder.h"
#include <algorithm>
namespace vll::freeplay { SessionAnalysis SessionAnalyzer::analyze(const std::vector<Sonority>& s,int voices)const{SessionAnalysis r;if(s.size()<2||voices<2||voices>4){r.error="Analysis requires at least two sonorities and 2-4 voices.";return r;}ExerciseConstraint c;c.voiceCount=voices;c.maximumLeap=24;analysis::VoiceAssigner a;auto b=visualization::VoicePathBuilder(a).build(s,c);if(!b.complete){r.error="Session voice paths are incomplete.";return r;}r.paths=b.paths;int common=0,steps=0,leaps=0;for(const auto&p:r.paths)for(std::size_t i=1;i<p.points.size();++i){int d=std::abs(p.points[i-1].pitch.distanceTo(p.points[i].pitch));common+=d==0;steps+=d==1||d==2;leaps+=d>2;}r.habits={{"common_tone",common},{"stepwise",steps},{"leap",leaps}};r.complete=true;return r;} }
