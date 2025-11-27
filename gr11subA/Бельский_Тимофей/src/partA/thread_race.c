#include <stdio.h>       // Подключение стандартной библиотеки ввода-вывода
#include <stdlib.h>      // Подключение библиотеки для работы с функциями malloc, atoi и т.д.
#include <pthread.h>     // Подключение библиотеки для работы с потоками POSIX
#include <stdatomic.h>   // Подключение библиотеки для атомарных операций
#include <time.h>        // Подключение библиотеки для работы со временем (clock())
#include <string.h>      // Подключение библиотеки для работы со строками (strcmp)

#define MODE_UNSYNC 0    // Режим без синхронизации
#define MODE_MUTEX 1     // Режим с использованием мьютексов
#define MODE_ATOMIC 2    // Режим с использованием атомарных операций

int counter = 0;                      // Глобальный счётчик (общий ресурс)
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Инициализация мьютекса
atomic_int atomic_counter = 0;        // Атомарный счётчик

// Функция, которую выполняют потоки
void* increment(void* arg) {
    int mode = *((int*)arg);          // Получаем режим работы из аргумента
    for (int i = 0; i < 1000000; i++) { // Каждый поток выполняет 1 миллион инкрементов
        if (mode == MODE_UNSYNC) {
            counter++;                // Без синхронизации: просто увеличиваем счётчик
        } else if (mode == MODE_MUTEX) {
            pthread_mutex_lock(&mutex); // Блокируем мьютекс для исключения гонки данных
            counter++;                // Увеличиваем счётчик
            pthread_mutex_unlock(&mutex); // Разблокируем мьютекс
        } else if (mode == MODE_ATOMIC) {
            atomic_fetch_add(&atomic_counter, 1); // Атомарное увеличение счётчика
        }
    }
    return NULL;                      // Завершаем выполнение потока
}

int main(int argc, char* argv[]) {
    // Проверяем количество аргументов командной строки
    if (argc != 4) {
        printf("Usage: %s <N> <M> <mode>\n", argv[0]); // Выводим инструкцию по использованию
        printf("Modes: unsync, mutex, atomic\n");      // Доступные режимы
        return 1;                                      // Завершаем программу с ошибкой
    }

    // Парсим аргументы командной строки
    int N = atoi(argv[1]); // Количество потоков
    int M = atoi(argv[2]); // Количество инкрементов на поток
    char* mode_str = argv[3]; // Режим работы (строка)
    int mode;

    // Определяем режим работы на основе переданной строки
    if (strcmp(mode_str, "unsync") == 0) {
        mode = MODE_UNSYNC; // Без синхронизации
    } else if (strcmp(mode_str, "mutex") == 0) {
        mode = MODE_MUTEX;  // С использованием мьютексов
    } else if (strcmp(mode_str, "atomic") == 0) {
        mode = MODE_ATOMIC; // С использованием атомарных операций
    } else {
        printf("Invalid mode. Use 'unsync', 'mutex', or 'atomic'.\n"); // Ошибка при неверном режиме
        return 1;                                                       // Завершаем программу с ошибкой
    }

    pthread_t threads[N]; // Массив идентификаторов потоков
    int mode_arg = mode;  // Передаваемый аргумент для потоков

    clock_t start = clock(); // Запоминаем время начала выполнения программы

    // Создаём N потоков
    for (int i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, increment, &mode_arg); // Создаём поток
    }

    // Ждём завершения всех потоков
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL); // Ожидаем завершения каждого потока
    }

    clock_t end = clock(); // Запоминаем время окончания выполнения программы
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC; // Вычисляем затраченное время

    int expected = N * M; // Ожидаемое значение счётчика
    int actual = (mode == MODE_ATOMIC) ? atomic_counter : counter; // Фактическое значение счётчика

    // Выводим результаты
    printf("Expected: %d\n", expected); // Ожидаемое значение
    printf("Actual: %d\n", actual);    // Фактическое значение
    printf("Time: %.6f seconds\n", time_spent); // Затраченное время

    return 0; // Завершаем программу успешно
}
