#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <iomanip>
#include <locale>

using namespace std;

mutex mtx; // Защита общей памяти при многопоточном вычислении

// Функция для формирования минора заданной матрицы
vector<vector<int>> extractMinor(const vector<vector<int>>& matrix, int excludeRow, int excludeCol) {
    vector<vector<int>> minorMatrix;
    int size = matrix.size();

    for (int i = 0; i < size; ++i) {
        if (i == excludeRow) continue;
        vector<int> rowElements;
        for (int j = 0; j < size; ++j) {
            if (j == excludeCol) continue;
            rowElements.push_back(matrix[i][j]);
        }
        minorMatrix.push_back(rowElements);
    }
    return minorMatrix;
}

// Функция для вычисления определителя
int computeDeterminant(const vector<vector<int>>& matrix) {
    int size = matrix.size();
    if (size == 1) return matrix[0][0];
    if (size == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    vector<thread> workers;
    vector<int> partialResults(size, 0);
    int detValue = 0;

    auto minorComputation = [&](int col) {
        vector<vector<int>> minorMatrix = extractMinor(matrix, 0, col);
        int coefficient = (col % 2 == 0) ? 1 : -1;
        int subDet = computeDeterminant(minorMatrix);
        mtx.lock();
        partialResults[col] = coefficient * matrix[0][col] * subDet;
        mtx.unlock();
        };

    for (int col = 0; col < size; ++col) {
        workers.emplace_back(minorComputation, col);
    }

    for (auto& worker : workers) {
        worker.join();
    }

    for (int result : partialResults) {
        detValue += result;
    }

    return detValue;
}

// Функция для красивого вывода матрицы
void printMatrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            cout << setw(5) << val << " ";
        }
        cout << endl;
    }
}

int main() {
    setlocale(LC_ALL, "Russian"); // Устанавливаем поддержку русского языка

    int dimension;
    cout << "Введите размер матрицы: ";
    cin >> dimension;

    vector<vector<int>> matrix(dimension, vector<int>(dimension));
    cout << "Введите элементы матрицы: " << endl;
    for (int i = 0; i < dimension; ++i) {
        for (int j = 0; j < dimension; ++j) {
            cin >> matrix[i][j];
        }
    }

    cout << "Введенная матрица: " << endl;
    printMatrix(matrix);

    int determinantResult = computeDeterminant(matrix);
    cout << "Определитель матрицы: " << determinantResult << endl;

    return 0;
}
