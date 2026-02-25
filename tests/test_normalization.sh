#!/bin/bash

# Configuration
SERVER_URL="http://localhost:4241"
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

printf "${GREEN}Starting URI Normalization & Security Tests...${NC}\n"
printf "Make sure your server is running on $SERVER_URL\n\n"

test_uri() {
    local method=$1
    local uri=$2
    local description=$3

    printf "Test: $description\n"
    printf "Sending: $method $uri\n"

    # Send request and capture status code
    response=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" --path-as-is "$SERVER_URL$uri")

    printf "Response Status: $response\n"
    printf "==========================\n"
}

# 1. Basic Normalization
test_uri "GET" "/./test/../index.html" "Basic dot-segment resolution (Expected: /index.html)"

# 2. Redundant Slashes
test_uri "GET" "/var//www/./" "Double slash and single dot (Expected: /var/www/)"

# 3. Directory Traversal (The 'Escape' Test)
test_uri "GET" "/../../../../etc/passwd" "Root escape attempt (Expected: / or 403/404)"

# 4. Complex Path
test_uri "GET" "/a/b/c/../../d/./e/../f" "Complex nested segments (Expected: /a/d/f)"

# 5. DELETE with Traversal
test_uri "DELETE" "/uploads/../config.conf" "DELETE escape attempt (Expected: 403 Forbidden)"

# 6. Trailing Dot
test_uri "GET" "/images/logo.png/." "Trailing dot (Expected: /images/logo.png/)"

printf "\n${GREEN}Tests Complete.${NC} Check your server logs to verify the 'before' and 'after' URI strings.\n"
