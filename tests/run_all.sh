#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCRIPT_DIR="$(dirname "$0")/scripts"

echo "========================================"
echo -e "${YELLOW}      WEBSERV FULL TEST SUITE${NC}"
echo "========================================"

run_test_step() {
    local script_name=$1
    local description=$2

    if [ -f "$SCRIPT_DIR/$script_name" ]; then
        bash "$SCRIPT_DIR/$script_name"
        echo "----------------------------------------"
    else
        echo -e "${RED}Error: $script_name not found in $SCRIPT_DIR!${NC}"
    fi
}

run_test_step "test_invalid_configs.sh" "Basic Invalid Configs"

run_test_step "test_duplicates.sh" "Duplicate Locations"

run_test_step "test_invalid_configs.sh" "Duplicate directive"

run_test_step "test_default_methods.sh" "Default methods not setted"

run_test_step "test_body_size.sh" "Body size with m, M, k and K"

echo -e "${GREEN}All test sequences completed!${NC}"
echo "========================================"