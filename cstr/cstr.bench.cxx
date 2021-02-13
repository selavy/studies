#include <benchmark/benchmark.h>

// TODO: package cstr correctly
// #include "cstr.h"
#include "cstr_header.h"

#include <vector>
#include <string>
#include <cstring>

void StringAppend(std::string& a, const std::string& b) { a += b; }
void StringAppend(cstr& a, const cstr& b) { cstr_append(&a, &b); }

void StringPrepend(std::string& a, const std::string& b) { a.insert(0, b); }
void StringPrepend(cstr& a, const cstr& b) { cstr_prepend(&a, &b); }

void StringInsert(std::string& a, std::size_t index, const std::string& b) { a.insert(index, b); }
void StringInsert(cstr& a, std::size_t index, const cstr& b) { cstr_insert(&a, index, &b); }

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
        String result = {};
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


template <class String>
static void BM_PrependSmallStrings(benchmark::State& state)
{
    std::vector<String> strings = {
        make<String>("a"),
        make<String>("e"),
        make<String>("cc"),
        make<String>("dd"),
    };
    int64_t count = 0;

    for (auto _ : state) {
        String result = {};
        for (auto&& s : strings) {
            StringPrepend(result, s);
        }
        count += StringSize(result);
    }

    if (count == 0) {
        throw std::runtime_error("invalid!");
    }
}
BENCHMARK_TEMPLATE(BM_PrependSmallStrings, cstr);
BENCHMARK_TEMPLATE(BM_PrependSmallStrings, std::string);

template <class String>
static void BM_CreationOverhead(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        String a = make<String>("Helld");
        String b = make<String>("lo, r");
        String c = make<String>("Wo");
        state.ResumeTiming();
    }
}
BENCHMARK_TEMPLATE(BM_CreationOverhead, cstr);
BENCHMARK_TEMPLATE(BM_CreationOverhead, std::string);

template <class String>
static void BM_InsertSmallStrings(benchmark::State& state)
{
    int64_t count = 0;
    for (auto _ : state) {
        state.PauseTiming();
        String a = make<String>("Helld");
        String b = make<String>("lo, r");
        String c = make<String>("Wo");
        state.ResumeTiming();
        StringInsert(a, 3, b);
        StringInsert(a, 7, c);
        count += StringSize(a);
    }

    if (count == 0) {
        throw std::runtime_error("invalid!");
    }
}
BENCHMARK_TEMPLATE(BM_InsertSmallStrings, cstr);
BENCHMARK_TEMPLATE(BM_InsertSmallStrings, std::string);


#define STRING_AND_SIZE_ARG(x) x, strlen(x)

static void BM_CreateSmallStrings_StdString(benchmark::State& state)
{
    int64_t count = 0;
    for (auto _ : state) {
        std::string a(STRING_AND_SIZE_ARG("Helld"));
        std::string b(STRING_AND_SIZE_ARG("lo, r"));
        std::string c(STRING_AND_SIZE_ARG("Wo"));
        std::string d(STRING_AND_SIZE_ARG("Hello, World"));
        std::string e(STRING_AND_SIZE_ARG("Hello, World"));
        count += StringSize(a);
        count += StringSize(b);
        count += StringSize(c);
        count += StringSize(d);
        count += StringSize(e);
    }

    if (count == 0) {
        throw std::runtime_error("invalid!");
    }
}
BENCHMARK(BM_CreateSmallStrings_StdString);

static void BM_CreateSmallStrings_CStr(benchmark::State& state)
{
    int64_t count = 0;
    for (auto _ : state) {
        cstr a, b, c, d, e;
        cstr_init(&a, STRING_AND_SIZE_ARG("Helld"));
        cstr_init(&b, STRING_AND_SIZE_ARG("lo, r"));
        cstr_init(&c, STRING_AND_SIZE_ARG("Wo"));
        cstr_init(&d, STRING_AND_SIZE_ARG("Hello, World"));
        cstr_init(&e, STRING_AND_SIZE_ARG("Hello, World"));
        benchmark::DoNotOptimize(count += StringSize(a));
        benchmark::DoNotOptimize(count += StringSize(b));
        benchmark::DoNotOptimize(count += StringSize(c));
        benchmark::DoNotOptimize(count += StringSize(d));
        benchmark::DoNotOptimize(count += StringSize(e));
    }

    if (count == 0) {
        throw std::runtime_error("invalid!");
    }
}
BENCHMARK(BM_CreateSmallStrings_CStr);

BENCHMARK_MAIN();
