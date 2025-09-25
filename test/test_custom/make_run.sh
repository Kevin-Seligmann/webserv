#!/bin/bash

ARG="$1"

make -C ../.. $ARG

cp -f ../../webserv conf/webserv.conf

./webserv conf/webserv.conf
