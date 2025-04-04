#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <iomanip>

const int N = 10000;

// Функция для генерации случайных чисел и заполнения массива
void fillArray(std::vector<int>& array) {
    for (auto& element : array) {
        element = rand() % 1000;
    }
}

// Последовательная версия сортировки методом нечётно-чётной транспозиции
void oddEvenSortSequential(std::vector<int>& array) {
    bool isSorted = false;
    int size = array.size();

    while (!isSorted) {
        isSorted = true;

        // Нечётная фаза
        for (int i = 1; i < size - 1; i += 2) {
            if (array[i] > array[i + 1]) {
                std::swap(array[i], array[i + 1]);
                isSorted = false;
            }
        }

        // Чётная фаза
        for (int i = 0; i < size - 1; i += 2) {
            if (array[i] > array[i + 1]) {
                std::swap(array[i], array[i + 1]);
                isSorted = false;
            }
        }
    }
}

// Параллельная версия сортировки методом нечётно-чётной транспозиции
void oddEvenSortParallel(std::vector<int>& array) {
    bool isSorted = false;
    int size = array.size();

    while (!isSorted) {
        isSorted = true;

        // Нечётная фаза
#pragma omp parallel for shared(array, isSorted)
        for (int i = 1; i < size - 1; i += 2) {
            if (array[i] > array[i + 1]) {
                std::swap(array[i], array[i + 1]);
#pragma omp critical
                isSorted = false;
            }
        }

        // Чётная фаза
#pragma omp parallel for shared(array, isSorted)
        for (int i = 0; i < size - 1; i += 2) {
            if (array[i] > array[i + 1]) {
                std::swap(array[i], array[i + 1]);
#pragma omp critical
                isSorted = false;
            }
        }
    }
}

int main() {
    // Настройка консоли для поддержки русского языка
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");

    omp_set_num_threads(4);

    std::vector<int> arraySequential(N);
    std::vector<int> arrayParallel(N);

    srand(static_cast<unsigned int>(time(nullptr)));

    std::cout << "=======================================================\n";
    std::cout << "           Сравнение производительности сортировок\n";
    std::cout << "     Метод нечётно-чётной транспозиции (Odd-Even Sort)\n";
    std::cout << "=======================================================\n";
    std::cout << "Размер массива: " << N << " элементов\n\n";

    // Заполнение массива
    fillArray(arraySequential);
    arrayParallel = arraySequential;

    // Последовательная сортировка
    auto start = std::chrono::high_resolution_clock::now();
    oddEvenSortSequential(arraySequential);
    auto end = std::chrono::high_resolution_clock::now();
    double timeSequential = std::chrono::duration<double>(end - start).count();

    std::cout << "Результат последовательной сортировки:\n";
    std::cout << "Время выполнения: " << std::fixed << std::setprecision(6) << timeSequential << " секунд\n\n";

    // Параллельная сортировка
    start = std::chrono::high_resolution_clock::now();
    oddEvenSortParallel(arrayParallel);
    end = std::chrono::high_resolution_clock::now();
    double timeParallel = std::chrono::duration<double>(end - start).count();

    std::cout << "Результат параллельной сортировки:\n";
    std::cout << "Время выполнения: " << std::fixed << std::setprecision(6) << timeParallel << " секунд\n\n";

    // Сравнительный анализ
    std::cout << "=======================================================\n";
    std::cout << "Сравнительный анализ:\n";
    std::cout << "Ускорение за счёт параллельной обработки: "
        << std::fixed << std::setprecision(2)
        << (timeSequential / timeParallel) << " раз(а)\n";
    std::cout << "=======================================================\n";

    return 0;
}
