#!/bin/bash

ARG="$1"

make clean -C ../..
make -C ../.. $ARG

cp -f ../../webserv webserv

chmod +x webserv

./webserv conf/webserv.conf
