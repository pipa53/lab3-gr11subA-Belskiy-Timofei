#include <stdio.h>      // Подключение стандартной библиотеки ввода-вывода
#include <stdlib.h>     // Подключение библиотеки для работы с функциями malloc, atoi и т.д.
#include <pthread.h>    // Подключение библиотеки для работы с потоками POSIX
#include <stdbool.h>    // Подключение библиотеки для работы с типом bool
#include <unistd.h>     // Подключение библиотеки для работы с sleep и другими системными вызовами
#include <string.h>     // Подключение библиотеки для работы со строками (strcmp)
#include <time.h>       // Подключение библиотеки для работы со временем

// Параметры программы
typedef struct {
    int buffer_size;       // Размер буфера
    int total_items;       // Общее количество элементов для производства
    int num_producers;     // Количество производителей
    int num_consumers;     // Количество потребителей
} Config;

// Глобальные переменные
int *buffer;              // Буфер (кольцевая очередь)
int count = 0;            // Текущее количество элементов в буфере
int in = 0, out = 0;      // Индексы для добавления и извлечения элементов
bool done = false;        // Флаг завершения работы
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Мьютекс для синхронизации доступа к буферу
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER; // Условная переменная: буфер не полон
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER; // Условная переменная: буфер не пуст

// Счётчики для статистики
int total_produced = 0;
int total_consumed = 0;
long long sum_produced = 0; // Сумма всех произведенных значений
long long sum_consumed = 0; // Сумма всех потребленных значений

// Функция производителя
void* producer(void* arg) {
    Config* config = (Config*)arg; // Получаем параметры конфигурации
    for (int i = 0; i < config->total_items / config->num_producers; i++) {
        pthread_mutex_lock(&mutex); // Блокируем мьютекс перед доступом к буферу
        while (count == config->buffer_size) {
            pthread_cond_wait(&not_full, &mutex); // Ждём, пока буфер не освободится
        }
        int value = rand() % 100; // Генерируем случайное число
        buffer[in] = value; // Производим элемент
        sum_produced += value; // Добавляем к сумме произведенных значений
        in = (in + 1) % config->buffer_size; // Перемещаем индекс ввода по кругу
        count++; // Увеличиваем количество элементов в буфере
        total_produced++; // Увеличиваем счётчик произведённых элементов
        pthread_cond_signal(&not_empty); // Уведомляем потребителей, что буфер не пуст
        pthread_mutex_unlock(&mutex); // Разблокируем мьютекс
    }
    return NULL;
}

// Функция потребителя
void* consumer(void* arg) {
    Config* config = (Config*)arg; // Получаем параметры конфигурации
    while (true) {
        pthread_mutex_lock(&mutex); // Блокируем мьютекс перед доступом к буферу
        while (count == 0 && !done) {
            pthread_cond_wait(&not_empty, &mutex); // Ждём, пока буфер не заполнится
        }
        if (done && count == 0) { // Если работа завершена и буфер пуст
            pthread_mutex_unlock(&mutex);
            break;
        }
        int item = buffer[out]; // Извлекаем элемент из буфера
        sum_consumed += item; // Добавляем к сумме потребленных значений
        out = (out + 1) % config->buffer_size; // Перемещаем индекс вывода по кругу
        count--; // Уменьшаем количество элементов в буфере
        total_consumed++; // Увеличиваем счётчик потреблённых элементов
        pthread_cond_signal(&not_full); // Уведомляем производителей, что буфер не полон
        pthread_mutex_unlock(&mutex); // Разблокируем мьютекс
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 9) { // Проверяем количество аргументов командной строки
        printf("Usage: %s -P <num_producers> -C <num_consumers> -N <total_items> -B <buffer_size>\n", argv[0]);
        return 1;
    }

    // Парсим аргументы командной строки
    Config config = {0};
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-P") == 0) {
            config.num_producers = atoi(argv[i + 1]); // Количество производителей
        } else if (strcmp(argv[i], "-C") == 0) {
            config.num_consumers = atoi(argv[i + 1]); // Количество потребителей
        } else if (strcmp(argv[i], "-N") == 0) {
            config.total_items = atoi(argv[i + 1]); // Общее количество элементов
        } else if (strcmp(argv[i], "-B") == 0) {
            config.buffer_size = atoi(argv[i + 1]); // Размер буфера
        }
    }

    // Проверяем, что все параметры заданы
    if (config.num_producers <= 0 || config.num_consumers <= 0 || config.total_items <= 0 || config.buffer_size <= 0) {
        printf("Invalid parameters. All values must be positive integers.\n");
        return 1;
    }

    // Инициализация буфера
    buffer = malloc(config.buffer_size * sizeof(int));
    if (!buffer) {
        perror("malloc");
        return 1;
    }

    // Замер времени выполнения
    clock_t start = clock();

    // Создание потоков
    pthread_t producers[config.num_producers];
    pthread_t consumers[config.num_consumers];

    for (int i = 0; i < config.num_producers; i++) {
        pthread_create(&producers[i], NULL, producer, &config); // Создаём потоки производителей
    }
    for (int i = 0; i < config.num_consumers; i++) {
        pthread_create(&consumers[i], NULL, consumer, &config); // Создаём потоки потребителей
    }

    // Ожидание завершения производителей
    for (int i = 0; i < config.num_producers; i++) {
        pthread_join(producers[i], NULL); // Ожидаем завершения каждого производителя
    }

    // Установка флага завершения и уведомление потребителей
    pthread_mutex_lock(&mutex);
    done = true;
    pthread_cond_broadcast(&not_empty); // Уведомляем всех потребителей
    pthread_mutex_unlock(&mutex);

    // Ожидание завершения потребителей
    for (int i = 0; i < config.num_consumers; i++) {
        pthread_join(consumers[i], NULL); // Ожидаем завершения каждого потребителя
    }

    // Окончание замера времени
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    // Проверка корректности
    bool is_count_correct = (total_produced == total_consumed && total_produced == config.total_items);
    bool is_sum_correct = (sum_produced == sum_consumed);
    bool is_correct = is_count_correct && is_sum_correct;

    // Вывод результатов
    printf("=== Producer-Consumer Results ===\n");
    printf("Configuration:\n");
    printf("  Producers: %d\n", config.num_producers);
    printf("  Consumers: %d\n", config.num_consumers);
    printf("  Total items: %d\n", config.total_items);
    printf("  Buffer size: %d\n", config.buffer_size);
    printf("\nStatistics:\n");
    printf("  Total produced: %d\n", total_produced);
    printf("  Total consumed: %d\n", total_consumed);
    printf("  Sum of produced values: %lld\n", sum_produced);
    printf("  Sum of consumed values: %lld\n", sum_consumed);
    printf("  Count correctness: %s\n", is_count_correct ? "CORRECT" : "INCORRECT");
    printf("  Sum correctness: %s\n", is_sum_correct ? "CORRECT" : "INCORRECT");
    printf("  Overall correctness: %s\n", is_correct ? "CORRECT" : "INCORRECT");
    printf("  Execution time: %.4f seconds\n", time_spent);

    // Освобождение ресурсов
    free(buffer);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}
