# Путь к исходникам для бенчмарков
get_filename_component(MONOLITH_BENCH_SOURCE_PATH "./${PROJECT_NAME}" ABSOLUTE)

# Базовое имя для целей бенчмарков
set(MONOLITH_BENCH_TARGET_BASE "${PROJECT_NAME}-bench")

# Макрос для создания исполнимых файлов для бенчмарков
macro(MONOLITH_BENCH_DECLARE suffix file)
    add_executable(
        ${MONOLITH_BENCH_TARGET_BASE}-${suffix}
        ${MONOLITH_BENCH_SOURCE_PATH}/${file}
    )

    # Подключаем необходимые библиотеки
    target_link_libraries(
        ${MONOLITH_BENCH_TARGET_BASE}-${suffix}
        ${PROJECT_NAME}
        benchmark::benchmark
    )
endmacro()

# Объявляем два бенчмарка
MONOLITH_BENCH_DECLARE(ema-sort-int ema-sort.cpp)
MONOLITH_BENCH_DECLARE(bin-search bin-search.cpp)

#генератор тестовых файлов
MONOLITH_BENCH_DECLARE(generator generator.cpp)

MONOLITH_BENCH_DECLARE(hello-world HelloWorldBench.cpp)
