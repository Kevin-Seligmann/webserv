make -C ../.. && mv ../../webserv .
echo "Youpi" > ./www/YoupiBanane/youpi.bla
./webserv ./conf/42.conf --interactive