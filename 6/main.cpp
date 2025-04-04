#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <windows.h>
#include <string>

const int HEIGHT = 30;
const int WIDTH = 80;
const int ITERATIONS = 100;
const int DELAY_MS = 100;

// Установка кодировки для отображения русского текста
void setupConsole() {
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Russian");
}

// Визуализация игрового поля
void renderField(const std::vector<std::vector<int>>& field) {
    system("cls");
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            std::cout << (field[i][j] ? "#" : ".");
        }
        std::cout << "\n";
    }
}

// Подсчёт живых соседей
int countNeighbors(const std::vector<std::vector<int>>& field, int x, int y) {
    int live = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < HEIGHT && ny >= 0 && ny < WIDTH) {
                live += field[nx][ny];
            }
        }
    }
    return live;
}

// Обновление игрового поля (параллельное)
void updateField(const std::vector<std::vector<int>>& current, std::vector<std::vector<int>>& next) {
#pragma omp parallel for collapse(2)
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            int neighbors = countNeighbors(current, i, j);
            if (current[i][j] == 1) {
                next[i][j] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            }
            else {
                next[i][j] = (neighbors == 3) ? 1 : 0;
            }
        }
    }
}

// Подсчёт живых клеток
int countLiveCells(const std::vector<std::vector<int>>& field) {
    int total = 0;
#pragma omp parallel for reduction(+:total)
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j < WIDTH; ++j)
            total += field[i][j];
    return total;
}

// Случайная инициализация
void initializeRandom(std::vector<std::vector<int>>& field) {
#pragma omp parallel for collapse(2)
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j < WIDTH; ++j)
            field[i][j] = rand() % 2;
}

// Ручная инициализация (пример шаблона)
void initializeGlider(std::vector<std::vector<int>>& field) {
    int x = HEIGHT / 2;
    int y = WIDTH / 2;
    field[x][y + 1] = 1;
    field[x + 1][y + 2] = 1;
    field[x + 2][y] = 1;
    field[x + 2][y + 1] = 1;
    field[x + 2][y + 2] = 1;
}

int main() {
    setupConsole();
    srand(static_cast<unsigned>(time(nullptr)));
    omp_set_num_threads(4);

    std::vector<std::vector<int>> field(HEIGHT, std::vector<int>(WIDTH));
    std::vector<std::vector<int>> nextField(HEIGHT, std::vector<int>(WIDTH));

    std::string initChoice;
    std::cout << "Выберите тип инициализации (random / glider): ";
    std::cin >> initChoice;

    if (initChoice == "glider") {
        initializeGlider(field);
    }
    else {
        initializeRandom(field);
    }

    for (int iter = 1; iter <= ITERATIONS; ++iter) {
        renderField(field);
        std::cout << "Итерация: " << iter << "\n";
        int liveCount = countLiveCells(field);
        std::cout << "Количество живых клеток: " << liveCount << "\n";

        updateField(field, nextField);
        field.swap(nextField);

        Sleep(DELAY_MS);
    }

    std::cout << "\nСимуляция завершена.\n";
    return 0;
}
