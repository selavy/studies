#include <benchmark/benchmark.h>

// TODO: package cstr correctly
#include "cstr.h"
// #include "cstr_header.h"
// #include "cstr_header2.h"

#include <vector>
#include <string>
#include <cstring>

void StringAppend(std::string& a, const std::string& b) { a += b; }
void StringAppend(cstr& a, const cstr& b) { cstr_append(&a, &b); }

auto StringSize(const std::string& a) { return a.size(); }
auto StringSize(const cstr&        a) { return cstr_len(&a); }

template <class T>
T make(const char* const s) { return T{s, strlen(s)}; };

template <> cstr make<cstr>(const char* const s)
{
    return cstr_make(s, strlen(s));
}

template <class String>
static void BM_AppendSmallStrings(benchmark::State& state) {
    std::vector<String> strings = {
        make<String>("a"),
        make<String>("e"),
        make<String>("cc"),
        make<String>("dd"),
    };
    int64_t count = 0;

    for (auto _ : state) {
        String result = make<String>("");
        for (auto&& s : strings) {
            StringAppend(result, s);
        }
        count += StringSize(result);
    }

    if (count == 0) {
        throw std::runtime_error("invalid!");
    }
}
BENCHMARK_TEMPLATE(BM_AppendSmallStrings, cstr);
BENCHMARK_TEMPLATE(BM_AppendSmallStrings, std::string);

BENCHMARK_MAIN();
