CXX ?= c++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -Werror -Iinclude -O2
BUILD_DIR := build
CORE_SOURCES := src/application/AppShell.cpp src/core/Logger.cpp src/core/Settings.cpp
APP_SOURCES := $(CORE_SOURCES) src/main.cpp
TEST_SOURCES := $(CORE_SOURCES) tests/TestMain.cpp tests/AppShellTests.cpp tests/SettingsTests.cpp tests/TypesTests.cpp

.PHONY: all test run clean

all: $(BUILD_DIR)/voice-leading-lab

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/voice-leading-lab: $(APP_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(APP_SOURCES) -o $@

$(BUILD_DIR)/vll_tests: $(TEST_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $@

test: $(BUILD_DIR)/vll_tests
	$(BUILD_DIR)/vll_tests

run: $(BUILD_DIR)/voice-leading-lab
	$(BUILD_DIR)/voice-leading-lab

clean:
	rm -f $(BUILD_DIR)/voice-leading-lab $(BUILD_DIR)/vll_tests
