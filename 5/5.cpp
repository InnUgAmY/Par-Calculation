#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <windows.h>

using namespace std;

// ������� ������������� ���������� ����� ��������� �������
long long parallelSum(const vector<int>& array) {
    long long sum = 0;

#pragma omp parallel for reduction(+:sum)
    for (size_t i = 0; i < array.size(); ++i) {
        sum += array[i];
    }

    return sum;
}

// ������� ����������������� ���������� �����
long long singleThreadSum(const vector<int>& array) {
    long long sum = 0;
    for (size_t i = 0; i < array.size(); ++i) {
        sum += array[i];
    }
    return sum;
}

int main() {
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");

    const size_t size = 10000000;
    vector<int> array(size);

    srand(static_cast<unsigned>(time(0)));
    for (size_t i = 0; i < size; ++i) {
        array[i] = rand() % 100;
    }

    omp_set_num_threads(4);

    cout << "\n--- ��������� ������� ���������� ����� ������� ---\n";

    auto start_time_parallel = chrono::high_resolution_clock::now();
    long long sum_parallel = parallelSum(array);
    auto end_time_parallel = chrono::high_resolution_clock::now();
    chrono::duration<double> time_parallel = end_time_parallel - start_time_parallel;

    cout << "������������ ���������� �����: " << sum_parallel << "\n";
    cout << "����� ����������: " << time_parallel.count() << " ������\n";
    cout << "��������: ������������ OpenMP � ���������, ����� �������� ����� ������.\n"
        << "������ ����� ������������ ���� ����� �������, ����� ���������� �����������.\n\n";

    auto start_time_single = chrono::high_resolution_clock::now();
    long long sum_single = singleThreadSum(array);
    auto end_time_single = chrono::high_resolution_clock::now();
    chrono::duration<double> time_single = end_time_single - start_time_single;

    cout << "���������������� ���������� �����: " << sum_single << "\n";
    cout << "����� ����������: " << time_single.count() << " ������\n";
    cout << "��������: ����� ������� � ����� ������ ��� ���������������.\n"
        << "������������������ ����, ��� ��� ��� ������ ����������� ���������������.\n\n";

    if (sum_parallel == sum_single) {
        cout << "���������� ���������! ���������� ��������� ���������.\n";
    }
    else {
        cout << "������: ���������� �����������! �������� �������� � ������������� ����.\n";
    }

    return 0;
}