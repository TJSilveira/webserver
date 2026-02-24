#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'


cd "$(dirname "$0")/../.."

echo "[STEP 2] Testing Duplicate Locations"

FILE="config/invalid/duplicate_location.conf"

if [ ! -f "$FILE" ]; then
	echo -e "${RED}Error: $FILE not found!${NC}"
	exit 1
fi

echo -n "   Checking duplicate location path: "
output=$(./webserv "$FILE" 2>&1)


if echo "$output" | grep -iq "duplicate"; then
	echo -e "${GREEN}PASS${NC} (Duplicate detected)"
else
	echo -e "${RED}FAIL${NC} (Server allowed duplicate locations!)"
fi