#include <mpi.h>
#include <iterator>
#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>
#include <numeric>

namespace MPI {

size_t ceil(unsigned x, unsigned y) {
	return 1 + (x - 1) / y;
}

template <typename ItTy>
auto sampleSort(ItTy Begin, ItTy End, const int CommSize) {
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
    std::transform(Buckets.begin(), Buckets.end(), BuckSzs.begin(), [](const auto &Bucket){
            return Bucket.size();
            });
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
    MPI_Alltoallv(BuckBuff.data(), BuckSzs.data(), Displs.data(), MPI_INT,
                  Res.get(), OtherSzs.get(), OtherDispls.data(), MPI_INT, MPI_COMM_WORLD);

    const auto ResBegin = Res.get();
    const auto ResEnd = ResBegin + ResSz;
    std::sort(ResBegin, ResEnd);
    return std::pair{std::move(Res), ResSz};
}

} // namespace MPI
