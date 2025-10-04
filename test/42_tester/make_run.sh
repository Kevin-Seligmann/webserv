if make -C ../..; then
    cp ../../webserv .
    ./webserv conf/42.conf
else
    rm -f webserv
    echo "Build failed."
    exit 1
fi