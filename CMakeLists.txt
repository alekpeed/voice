cmake_minimum_required(VERSION 3.20)
project(VoiceLeadingLab VERSION 0.1.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VLL_BUILD_TESTS "Build Voice Leading Lab tests" ON)
option(VLL_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(vll_core
    src/application/AppShell.cpp
    src/audio/AudioSample.cpp
    src/audio/LinuxAlsaOutput.cpp
    src/audio/MidiAudioRouter.cpp
    src/audio/SamplePiano.cpp
    src/audio/SfzPianoLoader.cpp
    src/audio/VelocityCurve.cpp
    src/core/Logger.cpp
    src/core/Settings.cpp
    src/midi/LinuxRawMidiInput.cpp
    src/midi/MidiByteStreamParser.cpp
    src/midi/MidiSession.cpp
    src/midi/VirtualMidiInput.cpp
)
target_include_directories(vll_core PUBLIC include)
target_link_libraries(vll_core PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
target_compile_options(vll_core PRIVATE -Wall -Wextra -Wpedantic)
if(VLL_WARNINGS_AS_ERRORS)
    target_compile_options(vll_core PRIVATE -Werror)
endif()

add_executable(voice-leading-lab src/main.cpp)
target_link_libraries(voice-leading-lab PRIVATE vll_core)

if(VLL_BUILD_TESTS)
    enable_testing()
    add_executable(vll_tests
        tests/TestMain.cpp
        tests/AppShellTests.cpp
        tests/AudioSampleTests.cpp
        tests/SamplePianoTests.cpp
        tests/VelocityCurveTests.cpp
        tests/MidiParserTests.cpp
        tests/MidiSessionTests.cpp
        tests/SettingsTests.cpp
        tests/TypesTests.cpp
    )
    target_link_libraries(vll_tests PRIVATE vll_core)
    target_compile_options(vll_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME vll_tests COMMAND vll_tests)
endif()

install(TARGETS voice-leading-lab RUNTIME DESTINATION bin)
