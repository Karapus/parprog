#pragma once
#include <numeric>
#include <numbers>
#include <boost/multiprecision/gmp.hpp>
#include <boost/math/special_functions/lambert_w.hpp>
#include <mpi.h>

namespace mpi_exp {

template <typename T>
struct ExportType;

template <typename T>
struct Exp {
    using value_type = T;
    T numer;
    T denom;
    Exp(T n, T d) : numer(std::move(n)), denom(std::move(d)) {}
    Exp(Exp low, Exp high) : numer(std::move(low.numer) * high.denom + std::move(high.numer)),
            denom(std::move(low.denom) * std::move(high.denom))
    {}
    template <typename F>
    auto get() const { return F{numer}/F{denom}; }
    static Exp binSplit(unsigned long a, unsigned long b) {
        if (b - a == 1) return Exp(1 /* x^b */, b);
        auto m = (a + b) / 2;
        return {binSplit(a, m), binSplit(m, b)};
    }
    ExportType<T> exportToMPI();
};

using mpz_int = boost::multiprecision::mpz_int;

template<>
struct ExportType<mpz_int> {
    using SizeTy = unsigned long;
    private:
    std::array<SizeTy, 2> Sizes;
    std::vector<std::byte> Data;
    public:
    ExportType(std::array<SizeTy, 2> SZ) : Sizes(SZ), Data(SZ[0] + SZ[1]) {}
    auto data() { return Data.data(); }
    void *sizeArr() { return Sizes.data(); }
    SizeTy size() { return Sizes[0] + Sizes[1]; }
    operator Exp<mpz_int>(){
        using namespace boost::multiprecision;
        mpz_int numer, denom;
        mpz_import(numer.backend().data(), Sizes[0], 1, sizeof(std::byte), 0, 0, Data.data());
        mpz_import(denom.backend().data(), Sizes[0], 1, sizeof(std::byte), 0, 0, Data.data() + Sizes[0]);
        return Exp{std::move(numer), std::move(denom)};
    }
};

template <>
ExportType<mpz_int> Exp<mpz_int>::exportToMPI() {
    using namespace boost::multiprecision;
    auto numer_src = numer.backend().data();
    auto denom_src = denom.backend().data();
    std::array<unsigned long, 2> SZs{mpz_sizeinbase(numer_src, 0xFF), mpz_sizeinbase(denom_src, 0xFF)};
    ExportType<mpz_int> res{SZs};
    mpz_export(res.data(),          nullptr, 1, sizeof(std::byte), 0, 0, numer_src);
    mpz_export(res.data() + SZs[0], nullptr, 1, sizeof(std::byte), 0, 0, denom_src);
    return res;
}

template <typename T>
Exp<T> collectExp(Exp<T> ThisExp, int Rank, int CommSize) {
    for (int Step = 1; Step < CommSize; Step *= 2) {
        if (Rank % (2 * Step)) {
            auto OtherRank = Rank - Step;
            auto Curnt = ThisExp.exportToMPI();
            MPI_Send(Curnt.sizeArr(), 2, MPI_UNSIGNED_LONG, OtherRank, 1, MPI_COMM_WORLD);
            MPI_Send(Curnt.data(), Curnt.size(), MPI_BYTE, OtherRank, 2, MPI_COMM_WORLD);
            break;
        } else {
            auto OtherRank = Rank + Step;
            std::array<unsigned long, 2> OtherSizeArr;
            MPI_Recv(OtherSizeArr.data(), 2, MPI_UNSIGNED_LONG, OtherRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            ExportType<T> Other{OtherSizeArr};
            MPI_Recv(Other.data(), Other.size(), MPI_BYTE, OtherRank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            ThisExp = Exp<T>{ThisExp, Other};
        }
    }
    return ThisExp;
}

inline unsigned long translateNDigsToNSumm(double n) {
    using namespace boost::multiprecision;
    return std::exp(1 + boost::math::lambert_w0(n * std::numbers::ln10 / std::numbers::e));
}

} // end namespace mpi_exp
