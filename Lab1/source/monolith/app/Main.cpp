#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>
#include <cstring>

namespace monolith::app {

    // Функция для измерения времени
    double GetTimeDifference(struct timeval start, struct timeval end) {
        double startSec = start.tv_sec + start.tv_usec / 1e6;
        double endSec = end.tv_sec + end.tv_usec / 1e6;
        return endSec - startSec;
    }

    // Функция для разбора ввода пользователя на команду и аргументы
    std::vector<std::string> ParseInput(const std::string& input) {
        std::vector<std::string> args;
        std::stringstream ss(input);
        std::string word;
        while (ss >> word) {
            args.push_back(word);
        }
        return args;
    }

    // Функция для выполнения команды cd
    void ChangeDirectory(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cerr << "Usage: cd <directory>" << std::endl;
            return;
        }

        // Получаем путь из аргумента
        std::string path = args[1];

        // Переход по пути ".." (родительский каталог)
        if (path == "..") {
            if (chdir("..") != 0) {
                std::cerr << "cd failed: " << strerror(errno) << std::endl;
            }
            return;
        }

        // Переход по относительному пути
        if (chdir(path.c_str()) != 0) {
            std::cerr << "cd failed: " << strerror(errno) << std::endl;
        }
    }

    // Функция для получения текущего пути
    std::string GetCurrentPath() {
        char buffer[1024];
        if (getcwd(buffer, sizeof(buffer)) != nullptr) {
            return std::string(buffer);
        } else {
            return "";
        }
    }

    // Основная функция для запуска программы
    void RunCommand(const std::string& command, const std::vector<std::string>& args) {
        pid_t pid = vfork();  // Используем vfork вместо fork
        
        if (pid == -1) {
            std::cerr << "vfork failed: " << strerror(errno) << std::endl;
            return;
        }

        if (pid == 0) { // Дочерний процесс
            std::vector<const char*> execArgs;
            execArgs.push_back(command.c_str());
            for (const auto& arg : args) {
                execArgs.push_back(arg.c_str());
            }
            execArgs.push_back(nullptr);  // Конец списка аргументов

            // Выполняем команду
            if (execvp(command.c_str(), const_cast<char* const*>(execArgs.data())) == -1) {
                std::cerr << "Exec failed: " << strerror(errno) << std::endl;
                _exit(1);  // Завершаем дочерний процесс через _exit, не через exit
            }
        } else {  // Родительский процесс
            struct timeval start, end;
            gettimeofday(&start, nullptr);  // Запоминаем время до запуска программы
            
            int status;
            waitpid(pid, &status, 0);  // Ждем завершения дочернего процесса
            
            gettimeofday(&end, nullptr);  // Запоминаем время после завершения
            
            // Выводим время выполнения программы
            double timeDiff = GetTimeDifference(start, end);
            std::cout << "Time taken: " << timeDiff << " seconds" << std::endl;
        }
    }

    // Функция для разбора ввода пользователя на команды (по пайпам)
    std::vector<std::string> SplitByPipe(const std::string& input) {
        std::vector<std::string> commands;
        std::stringstream ss(input);
        std::string command;
        while (std::getline(ss, command, '|')) {
            commands.push_back(command);
        }
        return commands;
    }

     // Функция для выполнения команд с пайпами
    void ExecutePipedCommands(const std::vector<std::string>& commands) {
        int numPipes = commands.size() - 1;
        std::vector<int> pipefds(2 * numPipes);  // Создаем вектор для пайпов

        // Создаем пайпы
        for (int i = 0; i < numPipes; ++i) {
            if (pipe(pipefds.data() + i * 2) == -1) {
                std::cerr << "Pipe failed: " << strerror(errno) << std::endl;
                return;
            }
        }

        std::vector<pid_t> pids;

        for (size_t i = 0; i < commands.size(); ++i) {
            pid_t pid = fork();
            if (pid == 0) {  // Дочерний процесс
                // Настраиваем ввод
                if (i != 0) {  // Не первая команда
                    dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
                }

                // Настраиваем вывод
                if (i != commands.size() - 1) {  // Не последняя команда
                    dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
                }

                // Закрываем все пайпы в дочернем процессе
                for (int j = 0; j < 2 * numPipes; ++j) {
                    close(pipefds[j]);
                }

                // Разбираем команду
                std::vector<std::string> args = ParseInput(commands[i]);
                std::vector<const char*> execArgs;
                for (const auto& arg : args) execArgs.push_back(arg.c_str());
                execArgs.push_back(nullptr);

                // Выполняем команду
                if (execvp(execArgs[0], const_cast<char* const*>(execArgs.data())) == -1) {
                    std::cerr << "Command failed: " << strerror(errno) << std::endl;
                    exit(1);
                }
            } else if (pid > 0) {
                pids.push_back(pid);
            } else {
                std::cerr << "Fork failed: " << strerror(errno) << std::endl;
                return;
            }
        }

        // Закрываем все пайпы в родительском процессе
        for (int i = 0; i < 2 * numPipes; ++i) {
            close(pipefds[i]);
        }

        // Ждем завершения всех дочерних процессов
        for (pid_t pid : pids) {
            waitpid(pid, nullptr, 0);
        }
    }
}  // namespace monolith::app

int main() {
    std::string input;
    while (true) {
        // Получаем текущую директорию
        std::string currentPath = monolith::app::GetCurrentPath();
        
        // Печатаем приглашение с текущим путем
        std::cout << currentPath << " $ ";  // Символ командной строки с текущим путем

        // Проверка на Ctrl+D
        if (!std::getline(std::cin, input)) {
            std::cout << "\nExiting shell due to EOF (Ctrl+D)..." << std::endl;
            break;  // Завершаем цикл
        }

        if (input.empty()) continue;  // Игнорировать пустые строки

        // Разделяем команды по символу '|'
        std::vector<std::string> pipedCommands = monolith::app::ParseInput(input);
        if (input.find('|') != std::string::npos) {  
            // Если есть пайпы, обрабатываем через ExecutePipedCommands
            std::vector<std::string> pipedCommands = monolith::app::SplitByPipe(input);
            monolith::app::ExecutePipedCommands(pipedCommands);
            continue;
        }

        // Разбор команды и аргументов
        std::vector<std::string> args = monolith::app::ParseInput(input);

        // Если введена команда "exit", завершаем цикл
        if (args[0] == "exit") {
            std::cout << "Exiting shell..." << std::endl;
            break;
        }

        // Если команда "cd", обрабатываем переход по каталогам
        if (args[0] == "cd") {
            monolith::app::ChangeDirectory(args);
        } else {
            // Запускаем команду
            monolith::app::RunCommand(args[0], std::vector<std::string>(args.begin() + 1, args.end()));
        }
    }
    return 0;
}
