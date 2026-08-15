#pragma once

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace vll::test {

using TestFunction = std::function<void()>;
std::vector<std::pair<std::string, TestFunction>>& registry();

struct Registrar {
    Registrar(std::string name, TestFunction function) {
        registry().emplace_back(std::move(name), std::move(function));
    }
};

template <typename Actual, typename Expected>
void requireEqual(const Actual& actual, const Expected& expected,
                  const char* actualText, const char* expectedText,
                  const char* file, const int line) {
    if (!(actual == expected)) {
        std::ostringstream message;
        message << file << ':' << line << " expected " << actualText
                << " == " << expectedText;
        throw std::runtime_error(message.str());
    }
}

inline void require(const bool condition, const char* expression,
                    const char* file, const int line) {
    if (!condition) {
        std::ostringstream message;
        message << file << ':' << line << " requirement failed: " << expression;
        throw std::runtime_error(message.str());
    }
}

} // namespace vll::test

#define VLL_JOIN_IMPL(a, b) a##b
#define VLL_JOIN(a, b) VLL_JOIN_IMPL(a, b)
#define TEST_CASE(name) \
    static void VLL_JOIN(test_, __LINE__)(); \
    static ::vll::test::Registrar VLL_JOIN(registrar_, __LINE__)(name, VLL_JOIN(test_, __LINE__)); \
    static void VLL_JOIN(test_, __LINE__)()
#define REQUIRE(expression) ::vll::test::require((expression), #expression, __FILE__, __LINE__)
#define REQUIRE_EQ(actual, expected) ::vll::test::requireEqual((actual), (expected), #actual, #expected, __FILE__, __LINE__)
