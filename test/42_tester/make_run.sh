rm -f webserv
make -C ../.. && cp -f ../../webserv .
./webserv ./conf/42b.conf --interactive