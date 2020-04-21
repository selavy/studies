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
#include "mafsa.h"
#include "mafsa2.h"


static const std::array<std::string, 3> DictionaryFilenames = {
    "csw19.ddic.gz",
    "csw19.tdic.gz",
    "csw19.mfsa.gz",
};
constexpr std::size_t DarrayDictionary = 0;
constexpr std::size_t TarrayDictionary = 1;
constexpr std::size_t  MafsaDictionary = 2;

static std::size_t countbytes()
{
    std::size_t result = 0;
    for (const auto& word : words) {
        result += word.size();
    }
    return result;
}

static const std::size_t total_word_bytes = countbytes();


template <class T, std::size_t DictFile>
static void BM_IsWord_AllWords(benchmark::State& state)
{
    static bool stats_dumped = false;
    auto maybe_darray = T::deserialize(DictionaryFilenames[DictFile]);
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
BENCHMARK_TEMPLATE(BM_IsWord_AllWords, Darray   , DarrayDictionary);
BENCHMARK_TEMPLATE(BM_IsWord_AllWords, Darray2  , DarrayDictionary);
BENCHMARK_TEMPLATE(BM_IsWord_AllWords, Tarraysep, TarrayDictionary);
BENCHMARK_TEMPLATE(BM_IsWord_AllWords, Tarray   , TarrayDictionary);
BENCHMARK_TEMPLATE(BM_IsWord_AllWords, Mafsa    ,  MafsaDictionary);
BENCHMARK_TEMPLATE(BM_IsWord_AllWords, Mafsa2   ,  MafsaDictionary);


BENCHMARK_MAIN();
