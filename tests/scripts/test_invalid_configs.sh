#!/bin/bash

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Running Configuration Validation Tests..."

# Array with config's names
configs=("invalid_max_body.conf" "invalid_listen.conf")

for conf in "${configs[@]}"
do
    echo -n "Testing $conf: "
    # Lainch server
    output=$(./webserv ./tests/config_validation/$conf 2>&1)
    
    # Check if is in output "Error" or "ConfigError"
    if echo "$output" | grep -iq "Error"; then
        echo -e "${GREEN}PASS${NC} (Caught invalid config)"
    else
        echo -e "${RED}FAIL${NC} (Server accepted invalid config!)"
    fi
done