CXX ?= c++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -Werror -Iinclude -O2 -pthread
LDLIBS ?= -ldl
BUILD_DIR := build
CORE_SOURCES := src/barry/BarryHarrisEngine.cpp src/ear/EarTrainingEngine.cpp \
	src/analysis/DeterministicFeedback.cpp src/analysis/VoiceAssigner.cpp \
	src/analysis/VoiceLeadingEvaluator.cpp \
	src/application/AppShell.cpp src/core/Logger.cpp src/core/Settings.cpp \
	src/curriculum/BookCurriculumEngine.cpp src/curriculum/CurriculumCatalog.cpp \
	src/curriculum/FundamentalCourse.cpp \
	src/exercise/DeterministicExerciseGenerator.cpp src/exercise/NearestVoicingSolver.cpp \
	src/harmony/MelodyHarmonizer.cpp src/lab/ReharmonizationLab.cpp \
	src/audio/AudioSample.cpp src/audio/InstrumentWorkshop.cpp src/audio/LinuxAlsaOutput.cpp src/audio/MidiAudioRouter.cpp \
	src/audio/SamplePiano.cpp src/audio/SfzPianoLoader.cpp src/audio/VelocityCurve.cpp \
	src/midi/LinuxRawMidiInput.cpp src/midi/MidiByteStreamParser.cpp \
	src/midi/MidiSession.cpp src/midi/VirtualMidiInput.cpp src/notation/NotationLayout.cpp \
	src/notation/SvgNotationRenderer.cpp src/performance/SonorityDetector.cpp \
	src/visualization/VisualizationModel.cpp src/visualization/VoiceGraphSvgRenderer.cpp \
	src/visualization/VoicePathBuilder.cpp
APP_SOURCES := $(CORE_SOURCES) src/main.cpp
TEST_SOURCES := $(CORE_SOURCES) tests/TestMain.cpp tests/AppShellTests.cpp tests/BarryHarrisTests.cpp tests/EarTrainingTests.cpp tests/MidiParserTests.cpp \
	tests/AudioSampleTests.cpp tests/MidiSessionTests.cpp tests/NotationTests.cpp tests/SamplePianoTests.cpp \
	tests/BookCurriculumEngineTests.cpp tests/CurriculumCatalogTests.cpp \
	tests/FeedbackTests.cpp tests/ExerciseGeneratorTests.cpp \
	tests/FundamentalCourseTests.cpp tests/InstrumentWorkshopTests.cpp \
	tests/MelodyHarmonizerTests.cpp tests/ReharmonizationLabTests.cpp \
	tests/SettingsTests.cpp tests/SonorityDetectorTests.cpp tests/TypesTests.cpp \
	tests/VelocityCurveTests.cpp tests/VoiceAssignmentTests.cpp tests/VisualizationTests.cpp

.PHONY: all test run clean

all: $(BUILD_DIR)/voice-leading-lab

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/voice-leading-lab: $(APP_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(APP_SOURCES) -o $@ $(LDLIBS)

$(BUILD_DIR)/vll_tests: $(TEST_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $@ $(LDLIBS)

test: $(BUILD_DIR)/vll_tests
	$(BUILD_DIR)/vll_tests

run: $(BUILD_DIR)/voice-leading-lab
	$(BUILD_DIR)/voice-leading-lab

clean:
	rm -f $(BUILD_DIR)/voice-leading-lab $(BUILD_DIR)/vll_tests
