#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <windows.h>

using namespace std;

const int matrixSize = 1000;
using Matrix = vector<vector<int>>;

// ���������� ������ ���������� �������
void fillMatrix(Matrix& matrix) {
    for (auto& row : matrix) {
        for (auto& element : row) {
            element = rand() % 100;
        }
    }
}

// ������� ������� (���������� ������)
void resetMatrix(Matrix& matrix) {
    for (auto& row : matrix) {
        fill(row.begin(), row.end(), 0);
    }
}

// ������� ��� ��������� ������� ����������
double measureExecutionTime(void (*func)(const Matrix&, const Matrix&, Matrix&), const Matrix& A, const Matrix& B, Matrix& C) {
    auto start = chrono::high_resolution_clock::now();
    func(A, B, C);
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration<double>(end - start).count();
}

// ���������������� ��������� ������ (��� ���������������)
void multiplySequential(const Matrix& A, const Matrix& B, Matrix& result) {
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            for (int k = 0; k < matrixSize; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// ������������ ��������� � OpenMP static
void multiplyParallelStatic(const Matrix& A, const Matrix& B, Matrix& result) {
#pragma omp parallel for schedule(static)
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            for (int k = 0; k < matrixSize; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// ������������ ��������� � OpenMP dynamic (��������� �� 4 �����)
void multiplyParallelDynamic(const Matrix& A, const Matrix& B, Matrix& result) {
#pragma omp parallel for schedule(dynamic, 4)
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            for (int k = 0; k < matrixSize; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");
    srand(static_cast<unsigned>(time(nullptr)));

    omp_set_num_threads(4);

    Matrix A(matrixSize, vector<int>(matrixSize));
    Matrix B(matrixSize, vector<int>(matrixSize));
    Matrix C(matrixSize, vector<int>(matrixSize, 0));

    fillMatrix(A);
    fillMatrix(B);

    cout << "\n--- ��������� ������� ��������� ������ ---\n";

    double timeSequential = measureExecutionTime(multiplySequential, A, B, C);
    cout << "���������������� ���������: " << timeSequential << " ���\n";
    cout << "��������: ������������ ���� �����, ���������� ���� ���������������.\n\n";

    resetMatrix(C);
    double timeStatic = measureExecutionTime(multiplyParallelStatic, A, B, C);
    cout << "OpenMP Static: " << timeStatic << " ���\n";
    cout << "��������: �������� ����� ���������� �������������� ����� ��������.\n";
    cout << "��� ����� ��������� � ���������� ��������, ���� ���������� ��������� �������� ���������� �� �������.\n\n";

    resetMatrix(C);
    double timeDynamic = measureExecutionTime(multiplyParallelDynamic, A, B, C);
    cout << "OpenMP Dynamic (4 �����): " << timeDynamic << " ���\n";
    cout << "��������: �������� �������������� ����������� �� 4 �����.\n";
    cout << "��� �������� �������� ���������� ��������, ���� ����� ���������� ��������� �������� �����������.\n";

    return 0;
}
