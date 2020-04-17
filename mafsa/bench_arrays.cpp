#include <benchmark/benchmark.h>
#include "darray.h"
#include <string>
#include <vector>

static const std::string darray_dictionary = "darray_csw19.bin";

// clang-format off
static const std::vector<std::string> words = {
    "JAMBIYAHS",
    "KAUMATUAS",
    "BEDRAL",
    "LEWIS",
    "SACERDOTALIST",
    "DODECAGONS",
    "KHANATES",
    "PHOTOELECTRICAL",
    "DROVERS",
    "RECEMENTED",
    "FURSHLUGGINER",
    "EMBLEMING",
    "TRANSPOSABLE",
    "MISSHAPES",
    "GELSEMINE",
    "SKIKJORERS",
    "BONDAGERS",
    "GLORIOSA",
    "SWATHIER",
    "MARACA",
    "CRIMINATORS",
    "HYPERREALIST",
    "SUK",
    "EXCELLENTLY",
    "GLEAMIEST",
    "MODISH",
    "KLUDGIER",
    "PELVIMETER",
    "NIGHTWATCHMAN",
    "AKEES",
    "RUMBLER",
    "TRIADICS",
    "PHOSPHORYLATIVE",
    "RUSTICISM",
    "ECHELONED",
    "TARAKIHIS",
    "SAVOURERS",
    "SPIROCHAETOSIS",
    "POTBOILER",
    "ANTIRABIES",
    "PEDALIER",
    "CIRCULATES",
    "UNSUBMITTING",
    "SUBURBIA",
    "DROPPERFULS",
    "VERMINIEST",
    "PIDGINISE",
    "BEGINS",
    "OCTOPODANS",
    "CARTELISTS",
    "SNELL",
    "MERCURIALIST",
    "CORPULENTLY",
    "OENOPHILE",
    "BUCKHORN",
    "PREFUNDED",
    "RADULAE",
    "GYROSCOPICALLY",
    "GRATINATE",
    "EMBRYOLOGIST",
    "AERODYNE",
    "TERFE",
    "PREDISCOVERY",
    "GNOSEOLOGIES",
    "SLUICE",
    "CRITICALITIES",
    "RENCOUNTERS",
    "PESSIMISMS",
    "BRITTLENESSES",
    "SLUNGSHOT",
    "PSYCHES",
    "ROSEWATER",
    "MONTICLES",
    "PICOGRAM",
    "SCRIMMAGER",
    "KILD",
    "GROUPIES",
    "CARBURISATIONS",
    "BRUSHMARKS",
    "OVERGRAZED",
    "KIRKYARD",
    "BATS",
    "RUMMELGUMPTIONS",
    "SHONKIEST",
    "WOLVED",
    "MISCALLING",
    "VERBIFICATIONS",
    "HYPERAEMIC",
    "ADRIFT",
    "VAMBRACES",
    "SMIRKED",
    "SYRPHID",
    "INTEL",
    "UPRISTS",
    "SPEARMEN",
    "SPYCAM",
    "FENI",
    "STEALTHINESSES",
    "BROILS",
    "RAMMISHNESS",
};
// clang-format on

static std::size_t countbytes()
{
    std::size_t result = 0;
    for (const auto& word : words) {
        result += word.size();
    }
    return result;
}

static const std::size_t total_word_bytes = countbytes();

static void BM_Darray_IsWord_AllWords(benchmark::State& state)
{
    auto maybe_darray = Darray::deserialize(darray_dictionary);
    if (!maybe_darray) {
        throw std::runtime_error("failed to deserialize darray!");
    }
    const auto& darray = *maybe_darray;
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
BENCHMARK(BM_Darray_IsWord_AllWords);

BENCHMARK_MAIN();
