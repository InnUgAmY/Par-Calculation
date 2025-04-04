#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <windows.h>
#include <iomanip>

using namespace std;

// Параллельное суммирование элементов массива
long long calculateSumParallel(const vector<int>& array) {
    long long sum = 0;

#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < array.size(); ++i) {
        sum += array[i];
    }

    return sum;
}

// Последовательное суммирование элементов массива
long long calculateSumSequential(const vector<int>& array) {
    long long sum = 0;

    for (int i = 0; i < array.size(); ++i) {
        sum += array[i];
    }

    return sum;
}

int main() {
    // Настройка кодировки и локали для корректного отображения русского текста
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");

    const size_t arraySize = 10000000;
    vector<int> array(arraySize);

    srand(static_cast<unsigned>(time(nullptr)));

    // Заполнение массива случайными числами от 0 до 99
    for (size_t i = 0; i < arraySize; ++i) {
        array[i] = rand() % 100;
    }

    // Установка числа потоков для OpenMP
    omp_set_num_threads(4);

    cout << "=============================================================\n";
    cout << "         Сравнение скорости параллельного и обычного\n";
    cout << "                суммирования элементов массива\n";
    cout << "=============================================================\n";
    cout << "Количество элементов в массиве: " << arraySize << "\n";
    cout << "Число потоков для параллельного режима: 4\n\n";

    // Параллельное суммирование
    auto startParallel = chrono::high_resolution_clock::now();
    long long sumParallel = calculateSumParallel(array);
    auto endParallel = chrono::high_resolution_clock::now();
    chrono::duration<double> timeParallel = endParallel - startParallel;

    // Последовательное суммирование
    auto startSequential = chrono::high_resolution_clock::now();
    long long sumSequential = calculateSumSequential(array);
    auto endSequential = chrono::high_resolution_clock::now();
    chrono::duration<double> timeSequential = endSequential - startSequential;

    // Результаты
    cout << "Результаты суммирования:\n";
    cout << " - Сумма (параллельно):     " << sumParallel << "\n";
    cout << " - Сумма (последовательно): " << sumSequential << "\n\n";

    cout << "Время выполнения:\n";
    cout << " - Параллельное выполнение:     " << fixed << setprecision(6) << timeParallel.count() << " секунд\n";
    cout << " - Последовательное выполнение: " << fixed << setprecision(6) << timeSequential.count() << " секунд\n\n";

    // Проверка корректности
    cout << "Проверка корректности:\n";
    if (sumParallel == sumSequential) {
        cout << " - Результаты совпадают. Суммирование выполнено корректно.\n";
    }
    else {
        cout << " - Ошибка: суммы не совпадают. Проверьте реализацию алгоритма.\n";
    }

    // Сравнение производительности
    cout << "\nСравнительный анализ:\n";
    cout << " - Параллельный способ оказался быстрее в "
        << fixed << setprecision(2)
        << (timeSequential.count() / timeParallel.count()) << " раз(а)\n";

    cout << "=============================================================\n";

    return 0;
}
