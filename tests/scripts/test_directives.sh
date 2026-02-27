#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

cd "$(dirname "$0")/../.."

echo -e "${YELLOW}[STEP 3] Testing Specific Directive Validation${NC}"

FILE="config/invalid/directive_overload.conf"

if [ ! -f "$FILE" ]; then
    echo -e "${RED}Error: $FILE not found!${NC}"
    exit 1
fi

output=$(./webserv "$FILE" 2>&1)

errors=("duplicate directive" "invalid error code" "duplicate location path found")

for err in "${errors[@]}"; do
    echo -n "   Checking for '$err': "
    if echo "$output" | grep -iq "$err"; then
        echo -e "${GREEN}PASS${NC}"
    else
        echo -e "${RED}FAIL${NC} (Expected error not found)"
    fi
done