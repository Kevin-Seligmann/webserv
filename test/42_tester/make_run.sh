rm -f webserv
make -C ../.. && cp -f ../../webserv .
./webserv ./conf/42.conf --interactive