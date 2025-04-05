#include <iostream>
#include <mpi.h>
#include <vector>
#include <ctime>
#include <chrono>

using namespace std;

// Функция для последовательного суммирования элементов массива
long long sequential_sum(const vector<int>& arr) {
    long long sum = 0;
    for (int i = 0; i < arr.size(); ++i) {
        sum += arr[i];
    }
    return sum;
}

// Функция для параллельного суммирования с использованием MPI
long long parallel_sum(const vector<int>& arr, int rank, int size) {
    int local_n = arr.size() / size;
    int remainder = arr.size() % size;
    long long local_sum = 0;

    int start = rank * local_n + (rank < remainder ? rank : remainder);
    int end = start + local_n + (rank < remainder ? 1 : 0);

    for (int i = start; i < end; ++i) {
        local_sum += arr[i];
    }

    long long global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    return global_sum;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Инициализация массива
    const int array_size = 4999999999999;
    vector<int> arr(array_size, 1); // Массив из единиц

    auto start = chrono::high_resolution_clock::now();

    if (rank == 0) {
        // Последовательное суммирование (выполняется только на процессе с rank == 0)
        auto seq_start = chrono::high_resolution_clock::now();
        long long seq_result = sequential_sum(arr);
        auto seq_end = chrono::high_resolution_clock::now();
        chrono::duration<double> seq_duration = seq_end - seq_start;
        cout << "Sequential sum: " << seq_result << endl;
        cout << "Sequential execution time: " << seq_duration.count() << " seconds." << endl;
    }

    // Параллельное суммирование
    auto par_start = chrono::high_resolution_clock::now();
    long long par_result = parallel_sum(arr, rank, size);
    auto par_end = chrono::high_resolution_clock::now();
    chrono::duration<double> par_duration = par_end - par_start;

    // Только процесс с rank == 0 выводит результаты
    if (rank == 0) {
        cout << "Parallel sum: " << par_result << endl;
        cout << "Parallel execution time: " << par_duration.count() << " seconds." << endl;

        // Сравнение времени выполнения
        cout << "Speedup: " << chrono::duration<double>(chrono::high_resolution_clock::now() - start).count() / par_duration.count() << "x" << endl;
    }

    MPI_Finalize();
    return 0;
}
