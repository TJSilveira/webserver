#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

cd "$(dirname "$0")/../.." || exit 1

echo -e "${YELLOW}Running Default Methods Tests from project root...${NC}"

run_check() {
	local conf=$1
	local port=$2
	local expected=$3
	local msg=$4

	pkill webserv 2>/dev/null 
	sleep 0.2

	./webserv "$conf" > server_test.log 2>&1 &
	local pid=$!
	
	sleep 0.2
	if ! kill -0 $pid 2>/dev/null; then
		echo -e "   $msg: ${RED}CRASHED${NC} (Check server_test.log)"
		cat server_test.log
		return 1
	fi

	sleep 0.8

	local code=$(curl -s -o /dev/null -w "%{http_code}" --http1.1 -H "Host: localhost" -H "Connection: close" http://127.0.0.1:$port/)
	
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

run_check "config/valid/default_get.conf" "4243" "200" "Default GET (no directive)"

run_check "config/valid/only_post.conf" "4244" "405" "Explicit POST only (GET restricted)"

rm -f server_test.log

echo -e "${YELLOW}Method tests finished.${NC}"