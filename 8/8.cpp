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
    const int array_size = 100000000; // Уменьшил размер для демонстрации
    vector<int> arr(array_size, 1); // Массив из единиц

    // Измеряем только время параллельного выполнения
    double seq_time = 0.0;
    double par_time = 0.0;

    if (rank == 0) {
        // Последовательное суммирование (выполняется только на процессе с rank == 0)
        auto seq_start = chrono::high_resolution_clock::now();
        long long seq_result = sequential_sum(arr);
        auto seq_end = chrono::high_resolution_clock::now();
        seq_time = chrono::duration<double>(seq_end - seq_start).count();
        cout << "Sequential sum: " << seq_result << endl;
        cout << "Sequential execution time: " << seq_time << " seconds." << endl;
    }

    // Синхронизация перед началом параллельного выполнения
    MPI_Barrier(MPI_COMM_WORLD);

    // Параллельное суммирование
    auto par_start = chrono::high_resolution_clock::now();
    long long par_result = parallel_sum(arr, rank, size);
    auto par_end = chrono::high_resolution_clock::now();
    par_time = chrono::duration<double>(par_end - par_start).count();

    // Только процесс с rank == 0 выводит результаты
    if (rank == 0) {
        cout << "Parallel sum: " << par_result << endl;
        cout << "Parallel execution time: " << par_time << " seconds." << endl;

        // Корректное сравнение времени выполнения
        if (seq_time > 0) {
            cout << "Speedup: " << seq_time / par_time << "x" << endl;
            cout << "Efficiency: " << (seq_time / par_time) / size * 100 << "%" << endl;
        }
    }

    MPI_Finalize();
    return 0;
}