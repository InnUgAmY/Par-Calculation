#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <windows.h>
#include <iomanip>

using namespace std;

// ������������ ������������ ��������� �������
long long calculateSumParallel(const vector<int>& array) {
    long long sum = 0;

#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < array.size(); ++i) {
        sum += array[i];
    }

    return sum;
}

// ���������������� ������������ ��������� �������
long long calculateSumSequential(const vector<int>& array) {
    long long sum = 0;

    for (int i = 0; i < array.size(); ++i) {
        sum += array[i];
    }

    return sum;
}

int main() {
    // ��������� ��������� � ������ ��� ����������� ����������� �������� ������
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");

    const size_t arraySize = 10000000;
    vector<int> array(arraySize);

    srand(static_cast<unsigned>(time(nullptr)));

    // ���������� ������� ���������� ������� �� 0 �� 99
    for (size_t i = 0; i < arraySize; ++i) {
        array[i] = rand() % 100;
    }

    // ��������� ����� ������� ��� OpenMP
    omp_set_num_threads(4);

    cout << "=============================================================\n";
    cout << "         ��������� �������� ������������� � ��������\n";
    cout << "                ������������ ��������� �������\n";
    cout << "=============================================================\n";
    cout << "���������� ��������� � �������: " << arraySize << "\n";
    cout << "����� ������� ��� ������������� ������: 4\n\n";

    // ������������ ������������
    auto startParallel = chrono::high_resolution_clock::now();
    long long sumParallel = calculateSumParallel(array);
    auto endParallel = chrono::high_resolution_clock::now();
    chrono::duration<double> timeParallel = endParallel - startParallel;

    // ���������������� ������������
    auto startSequential = chrono::high_resolution_clock::now();
    long long sumSequential = calculateSumSequential(array);
    auto endSequential = chrono::high_resolution_clock::now();
    chrono::duration<double> timeSequential = endSequential - startSequential;

    // ����������
    cout << "���������� ������������:\n";
    cout << " - ����� (�����������):     " << sumParallel << "\n";
    cout << " - ����� (���������������): " << sumSequential << "\n\n";

    cout << "����� ����������:\n";
    cout << " - ������������ ����������:     " << fixed << setprecision(6) << timeParallel.count() << " ������\n";
    cout << " - ���������������� ����������: " << fixed << setprecision(6) << timeSequential.count() << " ������\n\n";

    // �������� ������������
    cout << "�������� ������������:\n";
    if (sumParallel == sumSequential) {
        cout << " - ���������� ���������. ������������ ��������� ���������.\n";
    }
    else {
        cout << " - ������: ����� �� ���������. ��������� ���������� ���������.\n";
    }

    // ��������� ������������������
    cout << "\n������������� ������:\n";
    cout << " - ������������ ������ �������� ������� � "
        << fixed << setprecision(2)
        << (timeSequential.count() / timeParallel.count()) << " ���(�)\n";

    cout << "=============================================================\n";

    return 0;
}
