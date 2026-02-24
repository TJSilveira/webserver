#!/bin/bash

# Color
GREEN='\033[0;32m'
NC='\033[0m'

# Way to the folder with scripts
SCRIPT_DIR="$(dirname "$0")/scripts"

echo "========================================"
echo "      WEBSERV FULL TEST SUITE"
echo "========================================"

# Запуск теста конфигов
if [ -f "$SCRIPT_DIR/test_invalid_configs.sh" ]; then
    bash "$SCRIPT_DIR/test_invalid_configs.sh"
else
    echo "Error: test_invalid_configs.sh not found!"
fi

# Сюда в будущем добавишь: bash "$SCRIPT_DIR/test_cgi.sh"

echo "========================================"
echo -e "${GREEN}All test sequences completed!${NC}"