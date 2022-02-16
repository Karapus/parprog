#include <ranges>
#include <numeric>
#include <iostream>

namespace {

template <typename T, typename R>
T accumulate(R range, T init_val = T{}) {
	return std::accumulate(range.begin(), range.end(), init_val);
}

}

int main(int argc, char *argv[]) {
	int n = std::stoi(argv[1]);
	
	using namespace std::ranges::views;
	auto data_view = iota(1, n + 1) | transform([](int n){ return 1.0/n; });
	
	std::cout << accumulate<double>(data_view) << std::endl;
}
