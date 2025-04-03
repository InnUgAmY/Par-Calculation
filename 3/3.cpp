#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <mutex>
#include <iomanip>

const int NUM_THREADS = 10;
const int NUM_ITERATIONS = 100000;

// Глобальные переменные
int counter = 0;
std::mutex counterMutex;

// Функция инкрементации
void increment(bool useMutex) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        if (useMutex) {
            std::lock_guard<std::mutex> lock(counterMutex);
            counter++;
        }
        else {
            counter++;
        }
    }
}

void runTest(bool useMutex, const std::string& testName) {
    std::vector<std::thread> threads;
    counter = 0;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(increment, useMutex);
    }

    for (auto& t : threads) {
        t.join();
    }

    int expected = NUM_THREADS * NUM_ITERATIONS;
    std::cout << "\nТест: " << testName << "\n";
    std::cout << "Ожидаемое значение: " << expected << "\n";
    std::cout << "Фактическое значение: " << counter << "\n";

    if (!useMutex) {
        std::cout << "\nБез мьютекса:\n";
        std::cout << " - Потоки одновременно изменяют общее значение без синхронизации.\n";
        std::cout << " - Это приводит к гонке данных, из-за чего некоторые инкременты теряются.\n";
        std::cout << " - Итоговое значение обычно оказывается меньше ожидаемого.\n";
    }
    else {
        std::cout << "\nС мьютексом:\n";
        std::cout << " - Доступ к счетчику синхронизирован с помощью мьютекса.\n";
        std::cout << " - Каждый поток безопасно увеличивает значение.\n";
        std::cout << " - Итоговое значение совпадает с ожидаемым.\n";
    }
}

int main() {
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");
    std::cout << "\n--- Тестирование многопоточного инкремента счетчика ---\n";
    runTest(false, "Без мьютекса");
    runTest(true, "С мьютексом");
    return 0;
}
