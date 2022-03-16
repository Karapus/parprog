#include <iostream>
#include <string>
#include <iomanip>
#include <mpi.h>
#include "Exp.hh"

namespace {

unsigned long ceil(unsigned long x, unsigned long y) {
    return 1 + (x - 1) / y;
}

} // end namespace

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int Rank, CommSize;
    MPI_Comm_size(MPI_COMM_WORLD, &CommSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &Rank);

    if (argc != 2) { std::cerr << "Pass number of digits to calculate!" << std::endl; return 0; }
    auto n = std::stod(argv[1]) + 1;

    using namespace boost::multiprecision;
    using namespace mpi_exp;
    
    auto k = translateNDigsToNSumm(n);
    auto WorkSize = ceil(k, CommSize);
    auto ExpPart = Exp<mpz_int>::binSplit(WorkSize * Rank, WorkSize * (Rank + 1));
    auto ExpWout1 = collectExp(ExpPart, Rank, CommSize);
    
    mpf_float::default_precision(n * std::numbers::ln10 / std::numbers::ln2);
    if (!Rank)
        std::cout << std::setprecision(n) << 1 + ExpWout1.get<mpf_float>() << std::endl;
    MPI_Finalize();
}
