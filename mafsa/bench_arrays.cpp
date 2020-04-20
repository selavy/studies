#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include <iostream>
#include "bench_data.h"
#include "darray.h"
#include "darray2.h"
#include "tarray.h"
#include "tarraysep.h"
#include "tarraydelta.h"


static const std::string darray_dictionary = "csw19.ddic.gz";
static const std::string tarray_dictionary = "csw19.tdic.gz";

static std::size_t countbytes()
{
    std::size_t result = 0;
    for (const auto& word : words) {
        result += word.size();
    }
    return result;
}

static const std::size_t total_word_bytes = countbytes();


#if 1
template <class T>
static void BM_DarrayT_IsWord_AllWords(benchmark::State& state)
{
    static bool stats_dumped = false;
    auto maybe_darray = T::deserialize(darray_dictionary); // Darray::deserialize(darray_dictionary);
    if (!maybe_darray) {
        throw std::runtime_error("failed to deserialize darray!");
    }
    const auto& darray = *maybe_darray;
    if (!stats_dumped) {
        std::cout << "\n";
        darray.dump_stats(std::cout);
        std::cout << "\n";
        stats_dumped = true;
    }
    bool is_word = true;
    for (auto _ : state) {
        for (const auto& word : words) {
            is_word &= darray.isword(word);
        }
    }
    state.SetBytesProcessed(state.iterations() * total_word_bytes);
    if (!is_word) {
        throw std::runtime_error("test failed");
    }
}
BENCHMARK_TEMPLATE(BM_DarrayT_IsWord_AllWords, Darray);
BENCHMARK_TEMPLATE(BM_DarrayT_IsWord_AllWords, Darray2);
#endif


#if 1
static void BM_TarraySep_IsWord_AllWords(benchmark::State& state)
{
    static bool stats_dumped = false;
    auto maybe_tarray = Tarraysep::deserialize(tarray_dictionary);
    if (!maybe_tarray) {
        throw std::runtime_error("failed to deserialize darray!");
    }
    const auto& tarray = *maybe_tarray;
    if (!stats_dumped) {
        std::cout << "\n";
        tarray.dump_stats(std::cout);
        std::cout << "\n";
        stats_dumped = true;
    }
    bool is_word = true;
    for (auto _ : state) {
        for (const auto& word : words) {
            is_word &= tarray.isword(word);
        }
    }
    state.SetBytesProcessed(state.iterations() * total_word_bytes);
    if (!is_word) {
        throw std::runtime_error("test failed");
    }
}
BENCHMARK(BM_TarraySep_IsWord_AllWords);
#endif


#if 0
static void BM_TarrayDelta_IsWord_AllWords(benchmark::State& state)
{
    static bool stats_dumped = false;
    auto maybe_tarray = TarrayDelta::deserialize(tarray_dictionary);
    if (!maybe_tarray) {
        throw std::runtime_error("failed to deserialize darray!");
    }
    const auto& tarray = *maybe_tarray;
    if (!stats_dumped) {
        std::cout << "\n";
        tarray.dump_stats(std::cout);
        std::cout << "\n";
        stats_dumped = true;
    }
    bool is_word = true;
    for (auto _ : state) {
        for (const auto& word : words) {
            is_word &= tarray.isword(word);
        }
    }
    state.SetBytesProcessed(state.iterations() * total_word_bytes);
    if (!is_word) {
        throw std::runtime_error("test failed");
    }
}
BENCHMARK(BM_TarrayDelta_IsWord_AllWords);
#endif


#if 1
static void BM_TarrayCombo_IsWord_AllWords(benchmark::State& state)
{
    static bool stats_dumped = false;
    auto maybe_tarray = Tarray::deserialize(tarray_dictionary);
    if (!maybe_tarray) {
        throw std::runtime_error("failed to deserialize darray!");
    }
    const auto& tarray = *maybe_tarray;
    if (!stats_dumped) {
        std::cout << "\n";
        tarray.dump_stats(std::cout);
        std::cout << "\n";
        stats_dumped = true;
    }
    bool is_word = true;
    for (auto _ : state) {
        for (const auto& word : words) {
            is_word &= tarray.isword(word);
        }
    }
    state.SetBytesProcessed(state.iterations() * total_word_bytes);
    if (!is_word) {
        throw std::runtime_error("test failed");
    }
}
BENCHMARK(BM_TarrayCombo_IsWord_AllWords);
#endif


BENCHMARK_MAIN();
