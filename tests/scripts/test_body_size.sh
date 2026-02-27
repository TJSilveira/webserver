#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "--- Тестирование client_max_body_size (K/M) ---"

# 1. Запуск сервера
./webserv config/valid/test_size.conf > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

# Функция проверки
check_status() {
    local size=$1
    local expected=$2
    local url=$3
    
    # Создаем временный файл нужного размера
    head -c "$size" /dev/zero > temp_body.txt
    
    local code=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary @temp_body.txt "$url")
    
    if [ "$code" == "$expected" ]; then
        echo -e "Размер $size байт на $url: ${GREEN}OK (Код: $code)${NC}"
    else
        echo -e "Размер $size байт на $url: ${RED}FAIL (Ожидали $expected, получили $code)${NC}"
    fi
    rm temp_body.txt
}

# Тест 1: Лимит в локации /upload (10K = 10240 байт)
# 9КБ должно пройти (200 или 204 в зависимости от логики твоего POST)
check_status 9000 200 "http://127.0.0.1:4245/upload/"
# 11КБ должно вызвать ошибку 413
check_status 11000 413 "http://127.0.0.1:4245/upload/"

# Тест 2: Глобальный лимит (1M = 1048576 байт)
# 1.1МБ должно вызвать 413
check_status 1100000 413 "http://127.0.0.1:4245/"

# Завершение
kill $SERVER_PID
