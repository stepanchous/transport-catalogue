#pragma once
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "../src/geo.h"

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& container);

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& out,
                         const std::unordered_map<T1, T2>& container);

std::ostream& operator<<(std::ostream& out,
                         const trc::geo::Coordinates& coordintes);

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::tuple<T1, T2>& tup) {
    os << get<0>(tup) << ", " << get<1>(tup);
    return os;
}

template <typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, const std::tuple<T1, T2, T3>& tup) {
    os << get<0>(tup) << ", " << get<1>(tup) << ", " << get<2>(tup);
    return os;
}

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& out, const std::pair<T1, T2>& p) {
    std::cerr << p.first << ": " << p.second;
    return out;
}

template <typename Container>
void Print(std::ostream& out, const Container& container) {
    bool is_first = true;
    for (const auto& element : container) {
        if (is_first) {
            out << element;
            is_first = false;
        } else {
            out << ", " << element;
        }
    }
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& container) {
    std::cerr << "[";
    Print(out, container);
    std::cerr << "]";
    return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::set<T>& container) {
    std::cerr << "{";
    Print(out, container);
    std::cerr << "}";
    return out;
}

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& out, const std::map<T1, T2>& container) {
    std::cerr << "{";
    Print(out, container);
    std::cerr << "}";
    return out;
}

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& out,
                         const std::unordered_map<T1, T2>& container) {
    std::cerr << "{";
    Print(out, container);
    std::cerr << "}";
    return out;
}

void AssertImpl(bool value, const std::string& expr_str,
                const std::string& file, const std::string& func, unsigned line,
                const std::string& hint);

#define ASSERT(expr) \
    AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) \
    AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str,
                     const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line,
                     const std::string& hint) {
    using namespace std::string_literals;
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str
                  << ") failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) \
    AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) \
    AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename Test>
void RunTest(Test test, const std::string& test_name) {
    test();
    std::cerr << test_name << " OK" << std::endl;
}

#define RUN_TEST(test) RunTest((test), #test)
