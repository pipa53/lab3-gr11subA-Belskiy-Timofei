#!/bin/bash

# Создаем процесс, который будет работать 30 секунд
echo "Starting long-running process (sleep 30 seconds)..."
sleep 30 &
LONG_PID=$!
echo "PID of long-running process (sleep): $LONG_PID"

# Ждем 1 секунду, чтобы процесс точно запустился
sleep 1

# Проверяем, существует ли процесс
if kill -0 $LONG_PID 2>/dev/null; then
    echo "Process is running. Analyzing threads..."
    
    # Анализируем потоки этого процесса
    echo "=== Threads in ps ==="
    ps -L -p $LONG_PID -o pid,tid,psr,pcpu,stat,comm | head -n 20
    
    echo -e "\n=== Threads count from /proc/<PID>/status ==="
    cat /proc/$LONG_PID/status 2>/dev/null | grep Threads || echo "Cannot access /proc/$LONG_PID/status"
    
    echo -e "\n=== Threads in /proc/<PID>/task ==="
    ls -l /proc/$LONG_PID/task 2>/dev/null | head -n 10 || echo "Cannot access /proc/$LONG_PID/task"
else
    echo "ERROR: Process has already finished!"
    exit 1
fi

# Ждем завершения процесса
echo -e "\nWait for process to finish..."
wait $LONG_PID
echo "Analysis completed."
