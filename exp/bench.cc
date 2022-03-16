#include <benchmark/benchmark.h>
#include <chrono>
#include "Exp.hh"

namespace {

unsigned long ceil(unsigned long x, unsigned long y) {
	return 1 + (x - 1) / y;
}

// This reporter does nothing.
// We can use it to disable output from all but the root process
class NullReporter : public ::benchmark::BenchmarkReporter {
public:
  NullReporter() {}
  virtual bool ReportContext(const Context &) {return true;}
  virtual void ReportRuns(const std::vector<Run> &) {}
  virtual void Finalize() {}
};

}//namespace


static void BM_exp(benchmark::State& state) {
    int Rank, CommSize;
    MPI_Comm_size(MPI_COMM_WORLD, &CommSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
    for (auto _ : state) {
        auto n = state.range(0);

        auto start = std::chrono::high_resolution_clock::now();

        using namespace mpi_exp;
        auto k = translateNDigsToNSumm(n);
        auto WorkSize = ceil(k, CommSize);
        auto ExpPart = Exp<mpz_int>::binSplit(WorkSize * Rank, WorkSize * (Rank + 1));
        using mpf_float = boost::multiprecision::mpf_float;
#if 1
        benchmark::DoNotOptimize(ExpPart.get<mpf_float>());
#else
        auto ExpWout1 = collectExp(ExpPart, Rank, CommSize);
        benchmark::DoNotOptimize(ExpWout1.get<mpf_float>());
#endif
        auto end = std::chrono::high_resolution_clock::now();

        auto const duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        auto elapsed_seconds = duration.count();
        double max_elapsed_second;
        MPI_Allreduce(&elapsed_seconds, &max_elapsed_second, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
        state.SetIterationTime(max_elapsed_second);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_exp)->Range(1<<14, 1<<24)->Complexity()->UseManualTime()->Unit(benchmark::kMillisecond);;

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
