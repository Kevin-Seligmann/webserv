#!/bin/bash
ARG="$1"

rm -f webserv
rm -f ../../webserv
if make -C ../.. $ARG; then
    cp ../../webserv .
    ./webserv conf/42.conf
else
    echo "Build failed."
    exit 1
fi