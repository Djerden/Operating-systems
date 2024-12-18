#include <iostream>
#include <fstream>
#include <vector>

void printBinaryFile(const std::string& filename) {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return;
    }

    // Чтение данных из файла
    std::vector<int> buffer;
    int value;
    while (inputFile.read(reinterpret_cast<char*>(&value), sizeof(int))) {
        buffer.push_back(value);
    }

    // Вывод данных в консоль
    for (size_t i = 0; i < buffer.size(); ++i) {
        std::cout << buffer[i];
        if (i != buffer.size() - 1) {
            std::cout << ", ";  // Разделитель для удобства
        }
    }
    std::cout << std::endl;

    inputFile.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_file>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    printBinaryFile(filename);

    return 0;
}
