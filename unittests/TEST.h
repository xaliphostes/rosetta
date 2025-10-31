/*
 * Copyright (c) 2024-now fmaerten@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#pragma once
#include <rosetta/rosetta.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846 // Valeur de π si non définie
#endif

// -----------------------------------------------------------------------------------------------
//                   A la Google test framework: that is to say, GTEST using
//                   CMAKE
//                            Todo: Use TEST_CASE and SUB_CASE?
// -----------------------------------------------------------------------------------------------

// ======================================================
namespace test {

    using TestFunction = std::function<void()>;

    struct TestInfo {
        const char  *name;
        const char  *fixture;
        TestFunction fn;
    };

    inline std::vector<TestInfo> tests;

    inline void register_test(const char *name, const char *fixture, TestFunction fn) {
        tests.emplace_back(TestInfo{name, fixture, fn});
    }

} // namespace test

#define TEST(name, fixture)                                         \
    void name##_##fixture();                                        \
    static struct register_##name##_##fixture {                     \
        register_##name##_##fixture() {                             \
            test::register_test(#name, #fixture, name##_##fixture); \
        }                                                           \
    } register_##name##_##fixture##_instance;                       \
    void name##_##fixture()

// Declare a weak symbol
void initFct() __attribute__((weak));

#define RUN_TESTS()                                                                                \
    int main() {                                                                                   \
        if (initFct)                                                                               \
            initFct();                                                                             \
        for (const auto &test : test::tests) {                                                     \
            std::cout << "***************************************************"                     \
                         "*******"                                                                 \
                      << std::endl;                                                                \
            std::cout << "Running test [" << test.name << "/" << test.fixture << "]" << std::endl; \
            std::cout << "***************************************************"                     \
                         "*******"                                                                 \
                      << std::endl;                                                                \
            test.fn();                                                                             \
        }                                                                                          \
        return 0;                                                                                  \
    }
// ======================================================

// #define MSG(msg) std::cout << "---> " << msg << std::endl;

#define CHECK(condition)                                  \
    {                                                     \
        if (!(condition)) {                               \
            std::stringstream ss;                         \
            ss << "Check failed: " << #condition << "\n"; \
            ss << "File: " << __FILE__ << "\n";           \
            ss << "Line: " << __LINE__ << "\n";           \
            throw std::runtime_error(ss.str());           \
        }                                                 \
    }

#define EXPECT_EQ(val1, val2)                                                               \
    {                                                                                       \
        auto v1 = (val1);                                                                   \
        auto v2 = (val2);                                                                   \
        if (v1 != v2) {                                                                     \
            std::stringstream ss;                                                           \
            ss << "Expected " << v1 << " to equal " << v2 << " but |" << v1 << " != " << v2 \
               << "|";                                                                      \
            throw std::runtime_error(ss.str());                                             \
        }                                                                                   \
    }

#define EXPECT_GT(val1, val2)                                        \
    {                                                                \
        auto v1 = (val1);                                            \
        auto v2 = (val2);                                            \
        if (v1 <= v2) {                                              \
            std::stringstream ss;                                    \
            ss << "Expected " << v1 << " to be greater than " << v2; \
            throw std::runtime_error(ss.str());                      \
        }                                                            \
    }

#define EXPECT_GE(val1, val2)                                        \
    {                                                                \
        auto v1 = (val1);                                            \
        auto v2 = (val2);                                            \
        if (v1 < v2) {                                               \
            std::stringstream ss;                                    \
            ss << "Expected " << v1 << " to be greater than " << v2; \
            throw std::runtime_error(ss.str());                      \
        }                                                            \
    }

#define EXPECT_LT(val1, val2)                                     \
    {                                                             \
        auto v1 = (val1);                                         \
        auto v2 = (val2);                                         \
        if (v1 >= v2) {                                           \
            std::stringstream ss;                                 \
            ss << "Expected " << v1 << " to be less than " << v2; \
            throw std::runtime_error(ss.str());                   \
        }                                                         \
    }

#define EXPECT_LE(val1, val2)                                     \
    {                                                             \
        auto v1 = (val1);                                         \
        auto v2 = (val2);                                         \
        if (v1 > v2) {                                            \
            std::stringstream ss;                                 \
            ss << "Expected " << v1 << " to be less than " << v2; \
            throw std::runtime_error(ss.str());                   \
        }                                                         \
    }

// Expect not equal
#define EXPECT_NOT_EQ(val1, val2)                              \
    {                                                          \
        auto v1 = (val1);                                      \
        auto v2 = (val2);                                      \
        if (v1 == v2) {                                        \
            std::stringstream ss;                              \
            ss << "Expected " << v1 << " to not equal " << v2; \
            throw std::runtime_error(ss.str());                \
        }                                                      \
    }

#define EXPECT_NEAR(val1, val2, tol)                                                            \
    {                                                                                           \
        auto v1 = (val1);                                                                       \
        auto v2 = (val2);                                                                       \
        if (std::abs(v1 - v2) > tol) {                                                          \
            std::stringstream ss;                                                               \
            ss << "Expected " << v1 << " to be near " << v2 << " (tolerance=" << tol << ")"     \
               << " but |" << v1 << " - " << v2 << "| = " << std::abs(v1 - v2) << " > " << tol; \
            throw std::runtime_error(ss.str());                                                 \
        }                                                                                       \
    }

#define EXPECT_THROW(statement, exception_type)                                \
    {                                                                          \
        bool caught_expected = false;                                          \
        try {                                                                  \
            statement;                                                         \
        } catch (const exception_type &) {                                     \
            caught_expected = true;                                            \
        } catch (const std::exception &e) {                                    \
            std::stringstream ss;                                              \
            ss << "Expected " << #statement << " to throw " << #exception_type \
               << " but it threw a different exception: " << e.what();         \
            throw std::runtime_error(ss.str());                                \
        } catch (...) {                                                        \
            std::stringstream ss;                                              \
            ss << "Expected " << #statement << " to throw " << #exception_type \
               << " but it threw an unknown exception";                        \
            throw std::runtime_error(ss.str());                                \
        }                                                                      \
        if (!caught_expected) {                                                \
            std::stringstream ss;                                              \
            ss << "Expected " << #statement << " to throw " << #exception_type \
               << " but it didn't throw";                                      \
            throw std::runtime_error(ss.str());                                \
        }                                                                      \
    }

// String comparison
#define EXPECT_STREQ(str1, str2)                   \
    {                                              \
        std::string s1 = (str1);                   \
        std::string s2 = (str2);                   \
        if (s1 != s2) {                            \
            std::stringstream ss;                  \
            ss << "Expected strings to be equal\n" \
               << "    Got     : '" << s1 << "'\n" \
               << "    Expected: '" << s2 << "'";  \
            throw std::runtime_error(ss.str());    \
        }                                          \
    }

// Boolean condition true
#define EXPECT_TRUE(condition) CHECK(condition);

// Boolean condition false
#define EXPECT_FALSE(condition) CHECK(!(condition));

// No exception thrown
#define EXPECT_NO_THROW(statement)                                            \
    {                                                                         \
        try {                                                                 \
            statement;                                                        \
        } catch (const std::exception &e) {                                   \
            std::stringstream ss;                                             \
            ss << "Expected " << #statement << " not to throw, but it threw " \
               << "exception: " << e.what();                                  \
            throw std::runtime_error(ss.str());                               \
        } catch (...) {                                                       \
            std::stringstream ss;                                             \
            ss << "Expected " << #statement << " not to throw, but it threw " \
               << "an unknown exception";                                     \
            throw std::runtime_error(ss.str());                               \
        }                                                                     \
    }

template <typename Container1, typename Container2>
void expect_near(const Container1 &c1, const Container2 &c2, double tol) {
    if (c1.size() != c2.size()) {
        std::stringstream ss;
        ss << "Container sizes differ: " << c1.size() << " != " << c2.size();
        throw std::runtime_error(ss.str());
    }
    for (size_t i = 0; i < c1.size(); ++i) {
        if (std::abs(c1[i] - c2[i]) > tol) {
            std::stringstream ss;
            ss << "Containers differ at index " << i << ": " << c1[i] << " != " << c2[i]
               << " (diff = " << std::abs(c1[i] - c2[i]) << ", tolerance = " << tol << ")";
            throw std::runtime_error(ss.str());
        }
    }
}

#define EXPECT_ARRAY_NEAR(arr1, arr2, tol)                                                      \
    {                                                                                           \
        auto a1 = (arr1);                                                                       \
        auto a2 = (arr2);                                                                       \
        if (a1.size() != a2.size()) {                                                           \
            std::stringstream ss;                                                               \
            ss << "Array sizes differ: " << a1.size() << " != " << a2.size();                   \
            throw std::runtime_error(ss.str());                                                 \
        }                                                                                       \
        for (size_t i = 0; i < a1.size(); ++i) {                                                \
            if (std::abs(a1[i] - a2[i]) > tol) {                                                \
                std::stringstream ss;                                                           \
                ss << "Arrays differ at index " << i << ": " << a1[i] << " != " << a2[i]        \
                   << " (diff = " << std::abs(a1[i] - a2[i]) << ", tolerance = " << tol << ")"; \
                throw std::runtime_error(ss.str());                                             \
            }                                                                                   \
        }                                                                                       \
    }

#define EXPECT_ARRAY_EQ(arr1, arr2)                                                       \
    {                                                                                     \
        auto a1 = (arr1);                                                                 \
        auto a2 = (arr2);                                                                 \
        if (a1.size() != a2.size()) {                                                     \
            std::stringstream ss;                                                         \
            ss << "Array sizes differ: " << a1.size() << " != " << a2.size();             \
            throw std::runtime_error(ss.str());                                           \
        }                                                                                 \
        for (size_t i = 0; i < a1.size(); ++i) {                                          \
            if (a1[i] != a2[i]) {                                                         \
                std::stringstream ss;                                                     \
                ss << "Arrays differ at index " << i << ": " << a1[i] << " != " << a2[i]; \
                throw std::runtime_error(ss.str());                                       \
            }                                                                             \
        }                                                                                 \
    }

// Helper for timing measurements
template <typename F> double TIMING(F &&func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

#define CONTAINS(container1, container2)                                                     \
    {                                                                                        \
        for (const auto &item : container2) {                                                \
            if (std::find(container1.begin(), container1.end(), item) == container1.end()) { \
                std::stringstream ss;                                                        \
                ss << "Container does not contain expected item: " << item;                  \
                throw std::runtime_error(ss.str());                                          \
            }                                                                                \
        }                                                                                    \
    }
