#!/bin/bash

# Output color
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Go to project root
cd "$(dirname "$0")/../.."

echo -e "${YELLOW}Starting Invalid Configuration Tests...${NC}"

# Check for binary files
if [ ! -f "./webserv" ]; then
	echo -e "${RED}Error: webserv binary not found! Run make first.${NC}"
	exit 1
fi

# Tests count
PASSED=0
FAILED=0

for file in config/invalid/*.conf; do
	echo -n "Testing $(basename "$file"): "
	
	# Start server, catch stderr и stdout
	output=$(./webserv "$file" 2>&1)
	
	if [[ $? -ne 0 ]] || echo "$output" | grep -iq "Error"; then
		echo -e "${GREEN}PASS${NC} (Corrected rejected)"
		((PASSED++))
	else
		echo -e "${RED}FAIL${NC} (Server accepted invalid config!)"
		((FAILED++))
	fi
done

echo "---------------------------------------"
echo -e "Summary: ${GREEN}$PASSED passed${NC}, ${RED}$FAILED failed${NC}"