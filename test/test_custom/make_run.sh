#!/bin/bash
ARG="$1"
if make -C ../.. $ARG; then
    cp -f ../../webserv .
    ./webserv conf/webserv.conf
else
    rm -f webserv
    echo "Build failed."
    exit 1
fi