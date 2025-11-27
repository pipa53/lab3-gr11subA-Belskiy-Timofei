CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
TARGET = thread_race

all: $(TARGET)

$(TARGET): thread_race.c
	$(CC) $(CFLAGS) -o $(TARGET) thread_race.c

clean:
	rm -f $(TARGET)

run_unsync:
	./thread_race 4 1000000 unsync

run_mutex:
	./thread_race 4 1000000 mutex

run_atomic:
	./thread_race 4 1000000 atomic
