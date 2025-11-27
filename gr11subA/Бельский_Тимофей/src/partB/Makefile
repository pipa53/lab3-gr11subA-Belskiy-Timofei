# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
TARGET = prodcons

# Цель по умолчанию
all: $(TARGET)

# Сборка программы
$(TARGET): prodcons.c
	$(CC) $(CFLAGS) -o $(TARGET) prodcons.c

# Запуск программы с параметрами по умолчанию
run:
	./$(TARGET) -P 2 -C 2 -N 100000 -B 64

# Запуск программы с пользовательскими параметрами
run_custom:
	./$(TARGET) $(ARGS)

# Очистка скомпилированных файлов
clean:
	rm -f $(TARGET)

# Псевдо-цель (для удобства)
.PHONY: all run run_custom clean
