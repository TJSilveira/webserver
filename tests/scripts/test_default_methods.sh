#!/bin/bash

# Цвета для красивого вывода
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Исправление путей: переходим в корень проекта (на два уровня вверх от папки со скриптом)
cd "$(dirname "$0")/../.." || exit 1

echo -e "${YELLOW}Running Default Methods Tests from project root...${NC}"

# Функция для тестирования
run_check() {
	local conf=$1
	local port=$2
	local expected=$3
	local msg=$4

	# ОЧИСТКА: Убиваем старые серверы перед каждым тестом (die Bereinigung)
	pkill webserv 2>/dev/null 
	sleep 0.2

	# Запускаем сервер в фоне
	./webserv "$conf" > server_test.log 2>&1 &
	local pid=$!
	
	# Проверяем, жив ли процесс через 0.2 сек
	sleep 0.2
	if ! kill -0 $pid 2>/dev/null; then
		echo -e "   $msg: ${RED}CRASHED${NC} (Check server_test.log)"
		cat server_test.log
		return 1
	fi

	# Даем серверу время "проснуться" (die Verzögerung)
	sleep 0.8

	# Выполняем запрос через curl с заголовками для лучшей совместимости
	local code=$(curl -s -o /dev/null -w "%{http_code}" --http1.1 -H "Host: localhost" -H "Connection: close" http://127.0.0.1:$port/)
	
	# Завершаем процесс сервера (die Bereinigung)
	kill $pid 2>/dev/null
	wait $pid 2>/dev/null

	if [ "$code" == "$expected" ]; then
		echo -e "   $msg: ${GREEN}PASS${NC} (Code: $code)"
	else
		echo -e "   $msg: ${RED}FAIL${NC} (Expected $expected, got $code)"
		echo -e "      ${YELLOW}Hint:${NC} Try running 'curl -v http://127.0.0.1:$port/' while server is up."
		return 1
	fi
}

# 1. Проверка GET по умолчанию (директива allow_methods отсутствует)
run_check "config/valid/default_get.conf" "4243" "200" "Default GET (no directive)"

# 2. Проверка, что если указан только POST, GET выдает 405
run_check "config/valid/only_post.conf" "4244" "405" "Explicit POST only (GET restricted)"

# Убираем временный лог после завершения
rm -f server_test.log

echo -e "${YELLOW}Method tests finished.${NC}"