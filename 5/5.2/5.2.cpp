#include <iostream>
#include <omp.h>
#include <cmath>
#include <chrono>
#include <windows.h>
#include <iomanip>
#include <locale>

double integrateParallel(double a, double b, int steps) {
    double h = (b - a) / steps;
    double total = 0.0;

#pragma omp parallel for reduction(+:total)
    for (int i = 0; i < steps; ++i) {
        double x = a + (i + 0.5) * h;
        total += sin(x);
    }

    return total * h;
}

double integrateSingleThread(double a, double b, int steps) {
    double h = (b - a) / steps;
    double total = 0.0;

    for (int i = 0; i < steps; ++i) {
        double x = a + (i + 0.5) * h;
        total += sin(x);
    }

    return total * h;
}

int main() {
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");

    const double a = 0.0;
    const double b = 3.1415; // приближённое значение π
    const int steps = 10000000;
    const double expected = 2.0;
    const double epsilon = 1e-6;

    omp_set_num_threads(4);

    std::cout << "=============================================================\n";
    std::cout << "              Численное интегрирование функции\n";
    std::cout << "              Метод прямоугольников (Rectangle Rule)\n";
    std::cout << "=============================================================\n";
    std::cout << "Вычисляем определённый интеграл функции sin(x)\n";
    std::cout << "На интервале от " << a << " до " << b << "\n";
    std::cout << "Количество шагов: " << steps << "\n\n";

    auto start_parallel = std::chrono::high_resolution_clock::now();
    double result_parallel = integrateParallel(a, b, steps);
    auto end_parallel = std::chrono::high_resolution_clock::now();
    double time_parallel = std::chrono::duration<double>(end_parallel - start_parallel).count();

    auto start_single = std::chrono::high_resolution_clock::now();
    double result_single = integrateSingleThread(a, b, steps);
    auto end_single = std::chrono::high_resolution_clock::now();
    double time_single = std::chrono::duration<double>(end_single - start_single).count();

    std::cout << "-------------------- Результаты -----------------------------\n";
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Ожидаемое (аналитическое) значение интеграла : " << expected << "\n";
    std::cout << "Параллельный расчёт                          : " << result_parallel << "\n";
    std::cout << "Последовательный расчёт                      : " << result_single << "\n\n";

    std::cout << std::setprecision(6);
    std::cout << "Время выполнения (параллельно)   : " << time_parallel << " секунд\n";
    std::cout << "Время выполнения (один поток)    : " << time_single << " секунд\n";

    std::cout << "\n------------------ Проверка точности -------------------------\n";
    if (std::abs(result_parallel - expected) < epsilon &&
        std::abs(result_single - expected) < epsilon) {
        std::cout << "Оба метода дали корректный результат с допустимой погрешностью.\n";
    }
    else {
        std::cout << "Результаты не совпадают с аналитическим решением!\n";
    }

    std::cout << "=============================================================\n";
    std::cout << "Разница во времени выполнения (ускорение): "
        << std::setprecision(2) << time_single / time_parallel << " раз(а)\n";
    std::cout << "=============================================================\n";

    return 0;
}
