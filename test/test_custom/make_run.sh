#!/bin/bash

ARG="$1"

make -C ../.. $ARG

cp -f ../../webserv webserv

chmod +x webserv

./webserv conf/webserv.conf
