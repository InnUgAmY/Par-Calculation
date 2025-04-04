#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <windows.h>
#include <iomanip>

using namespace std;

// Последовательное умножение матрицы на вектор
vector<int> multiplyMatrixVectorSingle(const vector<vector<int>>& matrix, const vector<int>& vec) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    vector<int> result(rows, 0);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i] += matrix[i][j] * vec[j];
        }
    }
    return result;
}

// Параллельное умножение матрицы на вектор
vector<int> multiplyMatrixVectorParallel(const vector<vector<int>>& matrix, const vector<int>& vec) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    vector<int> result(rows, 0);

#pragma omp parallel for
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i] += matrix[i][j] * vec[j];
        }
    }
    return result;
}

int main() {
    // Настройка кодировки для поддержки русского языка
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");

    // Размерность матрицы и вектора
    const int rows = 5000, cols = 5000;
    omp_set_num_threads(8); // Количество потоков для OpenMP

    // Создание матрицы и вектора, заполненных единицами
    vector<vector<int>> matrix(rows, vector<int>(cols, 1));
    vector<int> vec(cols, 1);

    cout << "================================================================\n";
    cout << "                  Умножение матрицы на вектор                   \n";
    cout << "================================================================\n";
    cout << "Размерность матрицы: " << rows << " x " << cols << "\n";
    cout << "Размер вектора: " << cols << "\n";
    cout << "Каждый элемент матрицы и вектора равен 1.\n\n";

    // Последовательное умножение
    auto start_single = chrono::high_resolution_clock::now();
    vector<int> result_single = multiplyMatrixVectorSingle(matrix, vec);
    auto end_single = chrono::high_resolution_clock::now();
    chrono::duration<double> duration_single = end_single - start_single;

    // Параллельное умножение
    auto start_parallel = chrono::high_resolution_clock::now();
    vector<int> result_parallel = multiplyMatrixVectorParallel(matrix, vec);
    auto end_parallel = chrono::high_resolution_clock::now();
    chrono::duration<double> duration_parallel = end_parallel - start_parallel;

    // Вывод результатов
    cout << "------------------- Результаты вычислений ----------------------\n";
    cout << fixed << setprecision(6);
    cout << "Время выполнения (последовательно)   : " << duration_single.count() << " секунд\n";
    cout << "Время выполнения (параллельно)       : " << duration_parallel.count() << " секунд\n";

    // Оценка ускорения
    double speedup = duration_single.count() / duration_parallel.count();
    cout << "Ускорение за счёт параллельного вычисления: " << fixed << setprecision(2) << speedup << " раз(а)\n";

    cout << "================================================================\n";
    return 0;
}
