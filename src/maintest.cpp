
// REQUEST TEST


/*
    Use this to test Responses or CGI. Modify file 'request_test.txt'
    If the requests hangs is because it's not finished (For example, forgetting a LF after the headers)
*/

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <errno.h>
#include "HTTPRequest.hpp"
#include "RequestManager.hpp"
#include "ResponseManager.hpp"
#include "SysBufferFactory.hpp"

int main(int argc, char ** argv){
    int f = open("request_test.txt", O_RDONLY);

	if (f <= 0)
	{
		perror("request_test.txt");
		return 1;
	}

    HTTPRequest request;
    RequestManager manager(request, SysBufferFactory::SYSBUFF_FILE, f);

    while (1)
    {
        manager.process();
        if  (manager.gerError().status() != OK)
		{
			std::cout << "Fail" << std::endl;
			// Do anything with failed request
            break ;
		}
        if (manager.request_done())
		{
			std::cout << request;
			// Do anything with done request

			break ;
		}
    }
}