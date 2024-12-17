#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <algorithm>  // Для std::sort

// Константы для генерации данных
#define NUM_ELEMENTS 1000000
#define MAX_VALUE 10000  // Максимальное значение для чисел в файле

void generateBinarySearchFile(const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open output file for binary search!" << std::endl;
        return;
    }

    // Генерация случайных чисел для бинарного поиска
    std::vector<int> data(NUM_ELEMENTS);
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        data[i] = rand() % MAX_VALUE;  // Заполняем случайными числами
    }

    // Запись случайных данных в файл для бинарного поиска
    outFile.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(int));
    outFile.close();

    std::cout << "Binary file '" << filename << "' for binary search created!" << std::endl;
}

void generateSortFile(const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open output file for sorting!" << std::endl;
        return;
    }

    // Генерация случайных чисел для сортировки
    std::vector<int> data(NUM_ELEMENTS);
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        data[i] = rand() % MAX_VALUE;  // Заполняем случайными числами
    }

    // Сортируем данные перед записью в файл
    std::sort(data.begin(), data.end());

    // Запись отсортированных данных в файл для сортировки
    outFile.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(int));
    outFile.close();

    std::cout << "Binary file '" << filename << "' for sorting created!" << std::endl;
}

int main() {
    // Создание двух бинарных файлов
    generateBinarySearchFile("data_for_binary_search.bin");
    generateSortFile("data_for_sorting.bin");

    return 0;
}
