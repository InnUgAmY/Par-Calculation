#include <iostream>
#include <vector>
#include <ctime>
#include <mpi.h>
#include <cstdlib>
#include <iomanip>

// Функция для генерации случайной матрицы
std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
        }
    }
    return matrix;
}

// Функция для вывода части матрицы
void print_matrix_part(const std::vector<std::vector<double>>& matrix, int max_rows = 10, int max_cols = 10) {
    int rows = std::min(static_cast<int>(matrix.size()), max_rows);
    int cols = (matrix.empty()) ? 0 : std::min(static_cast<int>(matrix[0].size()), max_cols);

    std::cout << "Matrix (" << matrix.size() << "x"
        << (matrix.empty() ? 0 : matrix[0].size()) << "), showing "
        << rows << "x" << cols << ":\n";

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << std::fixed << std::setprecision(2) << matrix[i][j] << "\t";
        }
        std::cout << "\n";
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Размеры матриц (можно изменить или передавать через аргументы командной строки)
    int a_rows = 1000, a_cols = 1000;  // Матрица A: a_rows x a_cols
    int b_rows = a_cols, b_cols = 800;  // Матрица B: b_rows x b_cols (должно совпадать a_cols == b_rows)

    // Проверка на корректность размеров матриц
    if (a_cols != b_rows) {
        if (world_rank == 0) {
            std::cerr << "Error: The number of columns in matrix A must be equal to the number of rows in matrix B.\n";
        }
        MPI_Finalize();
        return 1;
    }

    std::vector<std::vector<double>> A, B, C;
    double start_time, end_time;

    // Главный процесс генерирует матрицы A и B
    if (world_rank == 0) {
        srand(static_cast<unsigned>(time(nullptr)));

        std::cout << "Generating matrices...\n";
        start_time = MPI_Wtime();

        A = generate_random_matrix(a_rows, a_cols);
        B = generate_random_matrix(b_rows, b_cols);
        C.resize(a_rows, std::vector<double>(b_cols, 0.0));

        std::cout << "Matrix A:\n";
        print_matrix_part(A);
        std::cout << "\nMatrix B:\n";
        print_matrix_part(B);
        std::cout << "\n";
    }

    // Рассылаем матрицу B всем процессам
    // Сначала рассылаем размеры B
    int b_dims[2];
    if (world_rank == 0) {
        b_dims[0] = b_rows;
        b_dims[1] = b_cols;
    }
    MPI_Bcast(b_dims, 2, MPI_INT, 0, MPI_COMM_WORLD);

    // Затем рассылаем саму матрицу B
    // Для оптимизации доступа к памяти преобразуем B в одномерный массив
    std::vector<double> B_linear(b_dims[0] * b_dims[1]);
    if (world_rank == 0) {
        for (int i = 0; i < b_dims[0]; ++i) {
            for (int j = 0; j < b_dims[1]; ++j) {
                B_linear[i * b_dims[1] + j] = B[i][j];
            }
        }
    }
    MPI_Bcast(B_linear.data(), b_dims[0] * b_dims[1], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Распределяем строки матрицы A между процессами
    int rows_per_process = a_rows / world_size;
    int remainder = a_rows % world_size;

    // Определяем, сколько строк получит текущий процесс
    int local_rows = rows_per_process + (world_rank < remainder ? 1 : 0);
    std::vector<double> A_local(local_rows * a_cols);

    // Массив с количеством строк для каждого процесса
    std::vector<int> counts(world_size);
    std::vector<int> displs(world_size);

    for (int i = 0; i < world_size; ++i) {
        counts[i] = (rows_per_process + (i < remainder ? 1 : 0)) * a_cols;
        displs[i] = (i == 0) ? 0 : displs[i - 1] + counts[i - 1];
    }

    // Главный процесс заполняет буфер для рассылки
    std::vector<double> A_linear;
    if (world_rank == 0) {
        A_linear.resize(a_rows * a_cols);
        for (int i = 0; i < a_rows; ++i) {
            for (int j = 0; j < a_cols; ++j) {
                A_linear[i * a_cols + j] = A[i][j];
            }
        }
    }

    // Разделяем матрицу A между процессами
    MPI_Scatterv(A_linear.data(), counts.data(), displs.data(), MPI_DOUBLE,
        A_local.data(), local_rows * a_cols, MPI_DOUBLE,
        0, MPI_COMM_WORLD);

    // Каждый процесс вычисляет свою часть результата
    std::vector<double> C_local(local_rows * b_cols, 0.0);

    for (int i = 0; i < local_rows; ++i) {
        for (int j = 0; j < b_cols; ++j) {
            double sum = 0.0;
            for (int k = 0; k < a_cols; ++k) {
                sum += A_local[i * a_cols + k] * B_linear[k * b_cols + j];
            }
            C_local[i * b_cols + j] = sum;
        }
    }

    // Собираем результаты на главном процессе
    // Сначала определяем counts и displs для результата
    for (int i = 0; i < world_size; ++i) {
        counts[i] = (rows_per_process + (i < remainder ? 1 : 0)) * b_cols;
        displs[i] = (i == 0) ? 0 : displs[i - 1] + counts[i - 1];
    }

    if (world_rank == 0) {
        // Преобразуем матрицу C в линейный вид для приема данных
        std::vector<double> C_linear(a_rows * b_cols);
        MPI_Gatherv(C_local.data(), local_rows * b_cols, MPI_DOUBLE,
            C_linear.data(), counts.data(), displs.data(), MPI_DOUBLE,
            0, MPI_COMM_WORLD);

        // Преобразуем линейный массив обратно в матрицу
        for (int i = 0; i < a_rows; ++i) {
            for (int j = 0; j < b_cols; ++j) {
                C[i][j] = C_linear[i * b_cols + j];
            }
        }

        end_time = MPI_Wtime();

        std::cout << "\nResult matrix C (partial view):\n";
        print_matrix_part(C);

        std::cout << "\n\nMatrix multiplication completed.\n";
        std::cout << "Time taken: " << end_time - start_time << " seconds\n";
    }
    else {
        MPI_Gatherv(C_local.data(), local_rows * b_cols, MPI_DOUBLE,
            nullptr, nullptr, nullptr, MPI_DOUBLE,
            0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}