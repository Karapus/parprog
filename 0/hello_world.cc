#include <mpi.h>
#include <iostream>

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);
	int commsize, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &commsize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	std::cout << "Hello World," << " Rank " << rank
            << " Size " << commsize << std::endl;
	MPI_Finalize();
}
