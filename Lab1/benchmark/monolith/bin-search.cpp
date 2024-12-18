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

bool binarySearch(const std::vector<int>& data, int target) {
    auto it = std::lower_bound(data.begin(), data.end(), target);
    return (it != data.end() && *it == target);
}

void externalBinarySearch(const std::string& filename, int target) {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return;
    }

    std::vector<int> buffer(BLOCK_SIZE / sizeof(int));
    while (inputFile.read(reinterpret_cast<char*>(buffer.data()), BLOCK_SIZE)) {
        size_t elementsRead = BLOCK_SIZE / sizeof(int);
        if (binarySearch(buffer, target)) {
            std::cout << "Element found!" << std::endl;
            inputFile.close();
            return;
        }
    }

    // Обработка оставшихся данных, если их меньше, чем BLOCK_SIZE
    size_t bytesRemaining = inputFile.gcount();  // Сколько байт было прочитано в последнем вызове
    if (bytesRemaining > 0) {
        size_t elementsRead = bytesRemaining / sizeof(int);
        buffer.resize(elementsRead);  // Уменьшаем размер буфера до количества прочитанных элементов
        if (binarySearch(buffer, target)) {
            std::cout << "Element found!" << std::endl;
            inputFile.close();
            return;
        }
    }

    std::cout << "Element not found!" << std::endl;
    inputFile.close();
}


void benchmark(int repetitions, const std::string& filename, int target, int numThreads) {
    struct timeval start, end;
    gettimeofday(&start, nullptr);

    // Разделение работы на потоки
    std::vector<std::thread> threads;
    for (int i = 0; i < repetitions; ++i) {
        if (threads.size() >= numThreads) {
            // Ожидаем завершения всех потоков, если их слишком много
            for (auto& t : threads) {
                t.join();
            }
            threads.clear();
        }
        // Запускаем новый поток для поиска
        threads.push_back(std::thread(externalBinarySearch, filename, target));
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
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <repetitions> <filename> <target> [threads]" << std::endl;
        return 1;
    }

    int repetitions = std::stoi(argv[1]);
    std::string filename = argv[2];
    int target = std::stoi(argv[3]);
    
    // Проверяем наличие параметра с количеством потоков
    int numThreads = 1;  // По умолчанию 1 поток (однопоточное выполнение)
    if (argc >= 5) {
        numThreads = std::stoi(argv[4]);  // Устанавливаем количество потоков из параметров
    }

    pid_t pid = vfork();
    if (pid == 0) {
        // Дочерний процесс, выполняет нагрузку
        benchmark(repetitions, filename, target, numThreads);
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
