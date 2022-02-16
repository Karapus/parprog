#include <ranges>
#include <numeric>
#include <iostream>
#include <mpi.h>

namespace {

template <std::ranges::range R>
auto accumulate(R range, std::ranges::range_value_t<R> init_val = {}) {
	return std::accumulate(range.begin(), range.end(), init_val);
}

auto ceil(auto x, auto y) {
	return 1 + (x - 1) / y;
}

}

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);

        if (argc == 1) {
            std::cerr << "Argument not passed!" << std::endl;
            MPI_Finalize();
            return 0;
        }

	auto n = std::stoi(argv[1]);

	using namespace std::ranges::views;
	auto whole_data_view = iota(1, n + 1) | transform([](int n){ return 1.0/n; });

	int commsize, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &commsize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	auto work_size = ceil(n, commsize);
	auto data_view = whole_data_view | drop(work_size * rank) | take(work_size);
	auto res = accumulate(data_view);

	decltype(res) sum;
	MPI_Reduce(&res, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	if (!rank) std::cout << sum << std::endl;
	MPI_Finalize();
}
