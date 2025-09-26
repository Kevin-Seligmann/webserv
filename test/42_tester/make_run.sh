rm webserv
if make -C ../..; then
    cp ../../webserv .
    ./webserv ./conf/42.conf --interactive
fi