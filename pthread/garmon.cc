#include <pthread.h>
#include <ranges>
#include <numeric>
#include <iostream>
#include <vector>

namespace {

template <std::ranges::range R>
auto accumulate(R range, std::ranges::range_value_t<R> init_val = {}) {
	return std::accumulate(range.begin(), range.end(), init_val);
}

auto ceil(auto x, auto y) {
	return 1 + (x - 1) / y;
}


std::vector<int> ThrdsData;
int N;
int NThrds;
std::vector<double> Sums;

void *pthreadAccumulate(void *arg) {
    auto Id = *static_cast<int *>(arg);
    using namespace std::ranges::views;
    auto WholeView = iota(1, N + 1) | transform([](int X){ return 1.0/X; });
    auto WorkSize = ceil(N, NThrds);
    auto MyView = WholeView | drop(WorkSize * Id) | take(WorkSize);
    Sums[Id] = accumulate(MyView);
    pthread_exit(NULL);
}
}

int main(int argc, char *argv[]) {
    if (argc != 3) return 0;
    N = std::stoi(argv[1]);
    NThrds = std::stoi(argv[2]);

    std::vector<pthread_t> Thrds(NThrds);
    ThrdsData.reserve(NThrds);
    Sums.resize(NThrds);
    for (auto i = 0; i < NThrds; ++i) {
        ThrdsData[i] = i;
        pthread_create(&Thrds[i], NULL, pthreadAccumulate, &ThrdsData[i]);
    }
    for (auto Thrd : Thrds)
        pthread_join(Thrd, NULL);
    std::cout << std::accumulate(Sums.begin(), Sums.end(), 0.0) << std::endl;
}
