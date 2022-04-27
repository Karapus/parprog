#include <pthread.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <numeric>

pthread_mutex_t CoutMutex = PTHREAD_MUTEX_INITIALIZER;

void *printHelloWorld(void *arg) {
    auto Id = *(int *)(arg);
    pthread_mutex_lock(&CoutMutex);
    std::cout << "Hello world! #" << Id << std::endl;
    pthread_mutex_unlock(&CoutMutex);
    pthread_exit(NULL);
}

std::vector<int> ThrdsData;

int main(int argc, char **argv) {
    auto NThrds = (argc == 2) ? std::stoi(argv[1]) : 0;

    std::vector<pthread_t> Thrds(NThrds);
    ThrdsData.reserve(NThrds);
    for (auto i = 0; i < NThrds; ++i) {
        ThrdsData[i] = i;
        pthread_create(&Thrds[i], NULL, printHelloWorld, &ThrdsData[i]);
    }
    for (auto Thrd : Thrds)
        pthread_join(Thrd, NULL);
}
