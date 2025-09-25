#!/bin/bash

./setup_test_env.sh

echo "=========================================="
echo "     WEBSERV COMPLETE TEST SUITE"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if server is running
check_server() {
    curl -s http://localhost:8080 > /dev/null 2>&1
    return $?
}

# Start server if not running
if ! check_server; then
    echo -e "${YELLOW}Starting webserv...${NC}"
    ./webserv conf/webserv.conf &
    SERVER_PID=$!
    sleep 2
    
    if ! check_server; then
        echo -e "${RED}Failed to start server!${NC}"
        exit 1
    fi
    echo -e "${GREEN}Server started with PID $SERVER_PID${NC}"
else
    echo -e "${GREEN}Server already running${NC}"
fi

echo ""

# Run all tests
echo "Running Basic Tests..."
python3 tests/test_basic.py
echo ""

echo "Running CGI Tests..."
python3 tests/test_cgi.py
echo ""

echo "Running Autoindex Tests..."
python3 tests/test_autoindex.py
echo ""

echo "Running Upload Tests..."
python3 tests/test_upload.py
echo ""

echo "Running Error Page Tests..."
python3 tests/test_errors.py
echo ""

echo "Running Method Tests..."
python3 tests/test_methods.py
echo ""

# Optional stress test
read -p "Run stress tests? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Running Stress Tests..."
    python3 tests/test_stress.py
fi

# Clean up
if [ ! -z "$SERVER_PID" ]; then
    echo ""
    echo -e "${YELLOW}Stopping server (PID $SERVER_PID)...${NC}"
    kill $SERVER_PID
fi

echo ""
echo "=========================================="
echo "     TEST SUITE COMPLETED"
echo "=========================================="