#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <mutex>

const int NUM_THREADS = 10;
const int NUM_ITERATIONS = 100000;

// ���������� ����������
int counter = 0;
std::mutex counterMutex;


// ������� �������������
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

	std::cout << testName << ": " << counter << " (��������� " << NUM_THREADS * NUM_ITERATIONS << ")\n";
}

int main() {
	SetConsoleOutputCP(65001);

	runTest(false, "Counter ��� ��������");
	runTest(true, "Counter � ���������");
	return 0;
}