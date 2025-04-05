#include <opencv2/opencv.hpp>
#include <omp.h>
#include <iostream>

using namespace std;
using namespace cv;

// Функция рисования ковра Серпинского
void drawSierpinski(Mat& image, int x, int y, int size, int depth) {
    if (depth == 0) {
        rectangle(image, Point(x, y), Point(x + size, y + size), Scalar(255, 255, 255), FILLED);
        return;
    }

    int newSize = size / 3;

    // Центр — оставить черным (не рисуем)
    for (int dx = 0; dx < 3; ++dx) {
        for (int dy = 0; dy < 3; ++dy) {
            if (dx == 1 && dy == 1) continue; // Пропускаем центр
            int nx = x + dx * newSize;
            int ny = y + dy * newSize;

            // Параллельный вызов
#pragma omp task firstprivate(nx, ny, newSize, depth)
            drawSierpinski(image, nx, ny, newSize, depth - 1);
        }
    }

#pragma omp taskwait
}

int main() {
    // Настройки
    const int imageSize = 729; // кратно 3^n (например, 3^6 = 729)
    const int maxDepth = 5;
    const string outputPath = "sierpinski_carpet.png";

    // Создаем черное изображение
    Mat image(imageSize, imageSize, CV_8UC3, Scalar(0, 0, 0));

    // Запускаем параллельную рекурсию
#pragma omp parallel
    {
#pragma omp single
        drawSierpinski(image, 0, 0, imageSize, maxDepth);
    }

    // Показываем результат
    imshow("Sierpinski Carpet", image);
    imwrite(outputPath, image);
    cout << "Изображение сохранено: " << outputPath << endl;

    waitKey(0);
    return 0;
}
