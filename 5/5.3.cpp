#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <windows.h>
#include <iomanip>

using namespace std;

// ���������������� ��������� ������� �� ������
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

// ������������ ��������� ������� �� ������
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
    // ��������� ��������� ��� ��������� �������� �����
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");

    // ����������� ������� � �������
    const int rows = 5000, cols = 5000;
    omp_set_num_threads(8); // ���������� ������� ��� OpenMP

    // �������� ������� � �������, ����������� ���������
    vector<vector<int>> matrix(rows, vector<int>(cols, 1));
    vector<int> vec(cols, 1);

    cout << "================================================================\n";
    cout << "                  ��������� ������� �� ������                   \n";
    cout << "================================================================\n";
    cout << "����������� �������: " << rows << " x " << cols << "\n";
    cout << "������ �������: " << cols << "\n";
    cout << "������ ������� ������� � ������� ����� 1.\n\n";

    // ���������������� ���������
    auto start_single = chrono::high_resolution_clock::now();
    vector<int> result_single = multiplyMatrixVectorSingle(matrix, vec);
    auto end_single = chrono::high_resolution_clock::now();
    chrono::duration<double> duration_single = end_single - start_single;

    // ������������ ���������
    auto start_parallel = chrono::high_resolution_clock::now();
    vector<int> result_parallel = multiplyMatrixVectorParallel(matrix, vec);
    auto end_parallel = chrono::high_resolution_clock::now();
    chrono::duration<double> duration_parallel = end_parallel - start_parallel;

    // ����� �����������
    cout << "------------------- ���������� ���������� ----------------------\n";
    cout << fixed << setprecision(6);
    cout << "����� ���������� (���������������)   : " << duration_single.count() << " ������\n";
    cout << "����� ���������� (�����������)       : " << duration_parallel.count() << " ������\n";

    // ������ ���������
    double speedup = duration_single.count() / duration_parallel.count();
    cout << "��������� �� ���� ������������� ����������: " << fixed << setprecision(2) << speedup << " ���(�)\n";

    cout << "================================================================\n";
    return 0;
}
