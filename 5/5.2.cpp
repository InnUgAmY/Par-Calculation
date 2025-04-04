#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <windows.h> // For SetConsoleOutputCP

// Parallel odd-even sort
void Sort(std::vector<int>& arr) {
    int n = arr.size();
    bool isSorted = false;

    while (!isSorted) {
        isSorted = true;

        // Even index elements
#pragma omp parallel for schedule(static) shared(arr) reduction(&& : isSorted)
        for (int i = 1; i < n - 1; i += 2) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }

        // Odd index elements
#pragma omp parallel for schedule(static) shared(arr) reduction(&& : isSorted)
        for (int i = 0; i < n - 1; i += 2) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
    }
}

// Single-threaded odd-even sort
void SortSolo(std::vector<int>& arr) {
    int n = arr.size();
    bool isSorted = false;

    while (!isSorted) {
        isSorted = true;
        for (int i = 1; i < n - 1; i += 2) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }

        for (int i = 0; i < n - 1; i += 2) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
    }
}

// Generate a random array
std::vector<int> generate(int n) {
    std::vector<int> result(n);
    for (int i = 0; i < n; i++) {
        result[i] = rand() % 200 - 100;
    }
    return result;
}

int main() {
    // Set console encoding to UTF-8 for proper text display
    SetConsoleOutputCP(65001);

    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);

    // Generate random array
    std::vector<int> arr = generate(100000);
    std::vector<int> arrSolo = arr;

    // Measure time for parallel sorting
    auto start = std::chrono::high_resolution_clock::now();
    Sort(arr);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "=Parallel sorting time: " << elapsed.count() << " seconds\n";

    // Measure time for single-threaded sorting
    start = std::chrono::high_resolution_clock::now();
    SortSolo(arrSolo);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;

    std::cout << "Single-threaded sorting time: " << elapsed.count() << " seconds\n";

    // Verify correctness of sorting
    if (arr == arrSolo) {
        std::cout << "Sorting results match!\n";
    }
    else {
        std::cout << "Error: Sorting results do not match!\n";
    }

    // Detailed explanation
    std::cout << "\n Final comparison:\n";
    std::cout << "1️ Parallel sorting uses multiple threads to speed up processing.\n";
    std::cout << "2️ Single-threaded sorting runs in one thread, making it slower.\n";
    std::cout << "3️ OpenMP (`#pragma omp parallel for`) distributes computation among threads.\n";
    std::cout << "4️ Conclusion: Multithreading is beneficial for large arrays.\n";

    return 0;
}
