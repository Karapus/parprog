#include <mpi.h>
#include <iostream>

namespace {

void sendMsg() {
    int rank, size, buff;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        std::cout << "Prcess 0 starts (nproc = " << size << ")" << std::endl;
        buff = 0;
        if (size == 1) return;
        MPI_Send(&buff, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
    
    auto sender = (rank ? rank : size) - 1;
    MPI_Recv(&buff, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    ++buff;
    std::cout << "Process " << rank << " recived " << buff << std::endl;
    
    if (!rank)
        return;

    auto recver = (rank + 1) % size;
    MPI_Send(&buff, 1, MPI_INT, recver, 0, MPI_COMM_WORLD);
}

}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    sendMsg();
    MPI_Finalize();
}
