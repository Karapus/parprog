#include <benchmark/benchmark.h>
#include <random>
#include <algorithm>

static void BM_SeqSort(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 g(rd());
    auto Size = state.range(0);
    std::vector<int> Workload(Size);
    std::iota(Workload.begin(), Workload.end(), 0);
    std::shuffle(Workload.begin(), Workload.end(), g);
    for (auto _ : state) {
        std::sort(Workload.begin(), Workload.end());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_SeqSort)->Range(1<<10, 1<<26)->Complexity()->Unit(benchmark::kMillisecond);
