#include "TestFramework.h"

#include <iostream>

namespace vll::test {

std::vector<std::pair<std::string, TestFunction>>& registry() {
    static std::vector<std::pair<std::string, TestFunction>> tests;
    return tests;
}

} // namespace vll::test

int main() {
    int failures = 0;
    for (const auto& [name, test] : vll::test::registry()) {
        try {
            test();
            std::cout << "PASS " << name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL " << name << ": " << error.what() << '\n';
        }
    }
    std::cout << vll::test::registry().size() << " tests, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
