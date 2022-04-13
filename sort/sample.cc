#include <mpi.h>
#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <memory>
#include <random>
#include <cassert>
#include <numeric>

namespace {

size_t ceil(unsigned x, unsigned y) {
	return 1 + (x - 1) / y;
}

std::random_device rd;
std::mt19937 g(rd());

template <typename ItTy>
auto sampleSort(ItTy Begin, ItTy End, const int CommSize, const int Rank) {
    const auto Size = End - Begin;
    auto Pivot = *(Begin + Size / 2);
    const auto PivSize = CommSize - 1;
    auto Pivots = std::make_unique<int[]>(CommSize);
    MPI_Allgather(&Pivot, 1, MPI_INT, Pivots.get(), 1, MPI_INT, MPI_COMM_WORLD);
    const auto PivBegin = Pivots.get();
    const auto PivEnd = PivBegin + PivSize;
    std::sort(PivBegin, PivEnd);

    auto Buckets = std::vector<std::vector<int>>(CommSize);
    std::for_each(Begin, End, [PivBegin, PivEnd, &Buckets](const auto &Val){
            Buckets[std::lower_bound(PivBegin, PivEnd, Val) - PivBegin].emplace_back(Val);
            });
    auto BuckSzs = std::vector<int>(CommSize);
    std::transform(Buckets.begin(), Buckets.end(), BuckSzs.begin(), [](const auto &Bucket){ return Bucket.size(); });
    auto OtherSzs = std::make_unique<int[]>(CommSize);
    const auto OtherSzsBegin = OtherSzs.get();
    const auto OtherSzsEnd = OtherSzsBegin + CommSize;
    MPI_Alltoall(BuckSzs.data(), 1, MPI_INT, OtherSzs.get(), 1, MPI_INT, MPI_COMM_WORLD);
    auto OtherDispls = std::vector<int>(CommSize);
    std::exclusive_scan(OtherSzsBegin, OtherSzsEnd, OtherDispls.begin(), 0);
    
    const auto ResSz = OtherDispls.back() + *(OtherSzsEnd - 1);
    auto Res = std::make_unique<int[]>(ResSz);
    auto Displs = std::vector<int>(CommSize);
    std::exclusive_scan(BuckSzs.begin(), BuckSzs.end(), Displs.begin(), 0);
    auto BuckBuff = std::vector<int>(Displs.back() + BuckSzs.back());
    std::for_each(Buckets.begin(), Buckets.end(), [It=BuckBuff.begin()](auto &Bucket) mutable {
            It = std::move(Bucket.begin(), Bucket.end(), It);
            });
    MPI_Alltoallv(BuckBuff.data(), BuckSzs.data(), Displs.data(), MPI_INT, Res.get(), OtherSzs.get(), OtherDispls.data(), MPI_INT, MPI_COMM_WORLD);
    const auto ResBegin = Res.get();
    const auto ResEnd = ResBegin + ResSz;
    std::sort(ResBegin, ResEnd);
    return std::pair{std::move(Res), ResSz};
}

} // namespace

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int CommSize, Rank;
    MPI_Comm_size(MPI_COMM_WORLD, &CommSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
    const int RootRank = 0;

    assert(argc == 2);
    std::vector<int> WholeWorkload(std::stoi(argv[1]));
    if (Rank == RootRank) {
        auto Begin = WholeWorkload.begin();
        auto End = WholeWorkload.end();
        std::iota(Begin, End, 0);
        std::shuffle(Begin, End, g);
    }
    const auto WlSize = ceil(WholeWorkload.size(), CommSize);
    auto Workload = std::make_unique<int[]>(WholeWorkload.size());
    auto WlBegin = Workload.get();
    auto WlEnd = WlBegin + std::min(WlSize, WholeWorkload.size() - Rank * WlSize);
    MPI_Scatter(WholeWorkload.data(), WlSize, MPI_INT,
                WlBegin, WlSize, MPI_INT,
                RootRank, MPI_COMM_WORLD);
    auto [Res, ResSz] = sampleSort(WlBegin, WlEnd, CommSize, Rank);
    const auto ResBegin = Res.get();
    const auto ResEnd = ResBegin + ResSz;
    assert(std::is_sorted(ResBegin, ResEnd));
    MPI_Finalize();
}
