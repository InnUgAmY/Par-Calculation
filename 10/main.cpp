#include <mpi.h>
#include <opencv2/opencv.hpp>
#include <iostream>

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int MAX_ITER = 1000;

// Функция проверки принадлежности к множеству Мандельброта
int mandelbrot(double x0, double y0) {
    double x = 0.0, y = 0.0;
    int iter = 0;
    while (x * x + y * y <= 4.0 && iter < MAX_ITER) {
        double xTemp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xTemp;
        iter++;
    }
    return iter;
}

// Преобразование итераций в красивый цвет
cv::Vec3b getColor(int iter) {
    if (iter == MAX_ITER) return cv::Vec3b(0, 0, 0);  // Чёрный для точек внутри множества

    double t = (double)iter / MAX_ITER;
    int r = static_cast<int>(9 * (1 - t) * t * t * t * 255);
    int g = static_cast<int>(15 * (1 - t) * (1 - t) * t * t * 255);
    int b = static_cast<int>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);

    return cv::Vec3b(b, g, r);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_proc = HEIGHT / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? HEIGHT : start_row + rows_per_proc;

    cv::Mat local_image(end_row - start_row, WIDTH, CV_8UC3);

    for (int row = start_row; row < end_row; row++) {
        for (int col = 0; col < WIDTH; col++) {
            double x0 = (col - WIDTH / 2.0) * 4.0 / WIDTH;
            double y0 = (row - HEIGHT / 2.0) * 4.0 / WIDTH;

            int iter = mandelbrot(x0, y0);
            local_image.at<cv::Vec3b>(row - start_row, col) = getColor(iter);
        }
    }

    cv::Mat final_image;
    if (rank == 0) {
        final_image.create(HEIGHT, WIDTH, CV_8UC3);
    }

    MPI_Gather(local_image.data, local_image.total() * local_image.elemSize(),
        MPI_UNSIGNED_CHAR, rank == 0 ? final_image.data : nullptr,
        local_image.total() * local_image.elemSize(), MPI_UNSIGNED_CHAR,
        0, MPI_COMM_WORLD);

    if (rank == 0) {
        cv::imshow("Mandelbrot Set — Colorful", final_image);
        cv::imwrite("mandelbrot_colored.png", final_image);
        cv::waitKey(0);
    }

    MPI_Finalize();
    return 0;
}
