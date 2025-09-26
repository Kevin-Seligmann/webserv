#!/bin/bash

ARG="$1"

make -C ../.. $ARG

cp -f ../../webserv webserv

chmod +x webserv

./webserv conf/webserv.conf > >(tee ../../logs/salida.log) 2> >(tee error.log >&2)
