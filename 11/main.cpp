#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>

// ==================== Глобальные переменные ====================
std::mutex frame_mutex;

// ==================== Функция обработки лица ====================
void process_face(const cv::Mat& gray, cv::Mat& frame, const cv::Rect& face,
    cv::CascadeClassifier& eyes_cascade, cv::CascadeClassifier& smile_cascade) {
    try {
        // Проверка границ лица
        if (face.x >= 0 && face.y >= 0 &&
            face.x + face.width <= gray.cols &&
            face.y + face.height <= gray.rows) {

            // Выделение области лица
            cv::Mat faceROI_gray = gray(face).clone();
            cv::Mat faceROI_color = frame(face).clone();

            // Детекция глаз
            std::vector<cv::Rect> eyes;
            eyes_cascade.detectMultiScale(faceROI_gray, eyes, 1.1, 10, 0, cv::Size(50, 50));

            // Детекция улыбок
            std::vector<cv::Rect> smiles;
            smile_cascade.detectMultiScale(faceROI_gray, smiles, 1.24, 15, 0, cv::Size(40, 40));

            // Блокировка доступа к кадру
            std::lock_guard<std::mutex> lock(frame_mutex);

            // Отрисовка прямоугольников
            if (face.x >= 0 && face.y >= 0 &&
                face.x + face.width <= frame.cols &&
                face.y + face.height <= frame.rows) {

                // Прямоугольник вокруг лица (синий)
                cv::rectangle(frame, face, cv::Scalar(255, 0, 0), 2);

                // Прямоугольники вокруг глаз (зеленые)
                for (const auto& eye : eyes) {
                    if (eye.x >= 0 && eye.y >= 0 &&
                        eye.x + eye.width <= faceROI_color.cols &&
                        eye.y + eye.height <= faceROI_color.rows) {
                        cv::rectangle(faceROI_color, eye, cv::Scalar(0, 255, 0), 2);
                    }
                }

                // Прямоугольники вокруг улыбок (красные)
                for (const auto& smile : smiles) {
                    if (smile.x >= 0 && smile.y >= 0 &&
                        smile.x + smile.width <= faceROI_color.cols &&
                        smile.y + smile.height <= faceROI_color.rows) {
                        cv::rectangle(faceROI_color, smile, cv::Scalar(0, 0, 255), 2);
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(frame_mutex);
        std::cerr << "Ошибка в потоке: " << e.what() << std::endl;
    }
}

// ==================== Главная функция ====================
int main() {
    try {
        // Загрузка каскадов Хаара
        cv::CascadeClassifier face_cascade, eyes_cascade, smile_cascade;
        if (!face_cascade.load("haarcascade_frontalface_default.xml") ||
            !eyes_cascade.load("haarcascade_eye.xml") ||
            !smile_cascade.load("haarcascade_smile.xml")) {
            std::cerr << "Ошибка загрузки каскадов Хаара!" << std::endl;
            return -1;
        }

        // Открытие видеофайла
        cv::VideoCapture cap("ZUA.mp4");
        if (!cap.isOpened()) {
            std::cerr << "Ошибка открытия видео!" << std::endl;
            return -1;
        }

        cv::Mat frame;
        int frame_count = 0;
        double total_single_thread = 0;
        double total_multi_thread = 0;

        // Обработка кадров видео
        while (cap.read(frame)) {
            if (frame.empty()) break;

            // Предварительная обработка изображения
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray, gray);
            cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

            // Детекция лиц
            std::vector<cv::Rect> faces;
            face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(150, 150));

            // Обработка в одном потоке (для сравнения)
            auto start_single = std::chrono::high_resolution_clock::now();
            for (const auto& face : faces) {
                process_face(gray, frame, face, eyes_cascade, smile_cascade);
            }
            auto end_single = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_single = end_single - start_single;
            total_single_thread += elapsed_single.count();

            // Обработка в нескольких потоках
            auto start_multi = std::chrono::high_resolution_clock::now();
            std::vector<std::thread> threads;
            for (const auto& face : faces) {
                threads.emplace_back(process_face,
                    std::cref(gray),
                    std::ref(frame),
                    std::cref(face),
                    std::ref(eyes_cascade),
                    std::ref(smile_cascade));
            }
            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
            auto end_multi = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_multi = end_multi - start_multi;
            total_multi_thread += elapsed_multi.count();

            frame_count++;

            // Вывод статистики каждые 10 кадров
            if (frame_count % 10 == 0) {
                double speedup = total_single_thread / (total_multi_thread + 1e-10);
                std::cout << "Кадр " << std::setw(4) << frame_count
                    << " | Ускорение: " << std::fixed << std::setprecision(2) << speedup << "x"
                    << " | Лица: " << faces.size()
                    << std::endl;
            }

            // Отображение результата
            cv::imshow("Детекция лиц", frame);
            if (cv::waitKey(30) >= 0) break;
        }

        // Итоговый отчет
        if (frame_count > 0) {
            double avg_speedup = total_single_thread / (total_multi_thread + 1e-10);
            std::cout << "\n=== ИТОГОВЫЙ ОТЧЕТ ===" << std::endl;
            std::cout << "Всего обработано кадров: " << frame_count << std::endl;
            std::cout << "Среднее ускорение:      " << std::fixed << std::setprecision(2) << avg_speedup << "x" << std::endl;
            std::cout << "Общее время (1 поток):  " << total_single_thread << " сек" << std::endl;
            std::cout << "Общее время (N потоков):" << total_multi_thread << " сек" << std::endl;
            std::cout << "=========================" << std::endl;
        }

        cap.release();
        cv::destroyAllWindows();
    }
    catch (const cv::Exception& e) {
        std::cerr << "Ошибка OpenCV: " << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception& e) {
        std::cerr << "Общая ошибка: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}