#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>
#include <cstring>
#include <thread>  // Для использования потоков

#define BLOCK_SIZE 1024 * 1024 * 32  // 32MB блок данных

// Функция сортировки слиянием
void mergeSort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    size_t mid = data.size() / 2;
    std::vector<int> left(data.begin(), data.begin() + mid);
    std::vector<int> right(data.begin() + mid, data.end());
    
    mergeSort(left);
    mergeSort(right);
    
    std::merge(left.begin(), left.end(), right.begin(), right.end(), data.begin());
}

// Функция сортировки данных из файла (работа с блоками)
void externalMergeSortBlock(const std::string& filename, size_t startPos, size_t blockSize, std::vector<int>& buffer) {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return;
    }

    inputFile.seekg(startPos);
    inputFile.read(reinterpret_cast<char*>(buffer.data()), blockSize);
    mergeSort(buffer);  // Сортируем текущий блок данных

    inputFile.close();
}

// Функция для выполнения сортировки с многозадачностью
void externalMergeSort(const std::string& filename) {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return;
    }

    // Получаем размер файла
    inputFile.seekg(0, std::ios::end);
    size_t fileSize = inputFile.tellg();
    inputFile.close();

    size_t blockSize = BLOCK_SIZE;
    if (fileSize < blockSize) {
        blockSize = fileSize;  // Если файл меньше блока, используем весь файл как один блок
    }

    std::vector<int> buffer(blockSize / sizeof(int));

    // Обрабатываем файл в одном потоке
    externalMergeSortBlock(filename, 0, fileSize, buffer);
}

// Функция для бенчмаркинга с многозадачностью
void benchmark(int repetitions, const std::string& filename, int numThreads) {
    struct timeval start, end;
    gettimeofday(&start, nullptr);

    // Разделяем повторения на несколько потоков
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread([=]() {
            for (int j = 0; j < repetitions / numThreads; ++j) {
                externalMergeSort(filename);
            }
        }));
    }

    // Если количество повторений не делится нацело на количество потоков, остаток выполняет основной поток
    int remainder = repetitions % numThreads;
    for (int i = 0; i < remainder; ++i) {
        externalMergeSort(filename);
    }

    // Ожидаем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    gettimeofday(&end, nullptr);
    double timeTaken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    std::cout << "Total time for " << repetitions << " repetitions: " << timeTaken << " seconds" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <repetitions> <filename> [threads]" << std::endl;
        return 1;
    }

    int repetitions = std::stoi(argv[1]);
    std::string filename = argv[2];

    // Проверяем наличие параметра с количеством потоков
    int numThreads = 1;  // По умолчанию 1 поток (однопоточное выполнение)
    if (argc >= 4) {
        numThreads = std::stoi(argv[3]);  // Устанавливаем количество потоков из параметров
    }

    pid_t pid = vfork();
    if (pid == 0) {
        // Дочерний процесс, выполняет нагрузку
        benchmark(repetitions, filename, numThreads);
        _exit(0);  // Завершаем дочерний процесс
    } else if (pid > 0) {
        // Родительский процесс
        waitpid(pid, nullptr, 0);  // Ждем завершения дочернего процесса
    } else {
        std::cerr << "vfork failed: " << strerror(errno) << std::endl;
        return 1;
    }

    return 0;
}
