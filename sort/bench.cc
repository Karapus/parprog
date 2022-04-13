#include <benchmark/benchmark.h>
#include <chrono>
#include <random>
#include "Sample.hh"

namespace {
// This reporter does nothing.
// We can use it to disable output from all but the root process
class NullReporter : public ::benchmark::BenchmarkReporter {
public:
  NullReporter() {}
  virtual bool ReportContext(const Context &) {return true;}
  virtual void ReportRuns(const std::vector<Run> &) {}
  virtual void Finalize() {}
};

double getTime(auto start, auto end) {
    auto const duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    auto elapsed_seconds = duration.count();
    double max_elapsed_second;
    MPI_Allreduce(&elapsed_seconds, &max_elapsed_second, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    return max_elapsed_second;
}

}//namespace


static void BM_MPISort(benchmark::State& state) {
    int Rank, CommSize;
    MPI_Comm_size(MPI_COMM_WORLD, &CommSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
    std::random_device rd;
    std::mt19937 g(rd());
    for (auto _ : state) {
        auto Size = state.range(0);
        const auto AvgSize = MPI::ceil(Size, CommSize);
        const auto WlSize = std::min(AvgSize,
                                     Size - Rank * AvgSize);
        std::vector<int> Workload(WlSize);
        std::iota(Workload.begin(), Workload.end(), Rank * AvgSize);
        std::shuffle(Workload.begin(), Workload.end(), g);
        auto start = std::chrono::high_resolution_clock::now();
        auto [Res, ResSz] = MPI::sampleSort(Workload.begin(), Workload.end(), CommSize);
        auto end = std::chrono::high_resolution_clock::now();
        const auto ResBegin = Res.get();
        const auto ResEnd = ResBegin + ResSz;
        assert(std::is_sorted(ResBegin, ResEnd));
        state.SetIterationTime(getTime(start, end));
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_MPISort)->Range(1<<10, 1<<26)->Complexity()->UseManualTime()->Unit(benchmark::kMillisecond);;

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  ::benchmark::Initialize(&argc, argv);

  if(rank == 0)
    // root process will use a reporter from the usual set provided by
    // ::benchmark
    ::benchmark::RunSpecifiedBenchmarks();
  else {
    // reporting from other processes is disabled by passing a custom reporter
    NullReporter null;
    ::benchmark::RunSpecifiedBenchmarks(&null);
  }

  MPI_Finalize();
}
