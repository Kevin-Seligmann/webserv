
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

void generate_rq_response(ResponseManager & responseManager)
{
	responseManager.generate_response();
	while (1)
	{
		// responseManager.get_active_file_descriptor();
		responseManager.process();
		if (responseManager.response_done())
			break ;
	}
}

int main(int argc, char ** argv){
    int f = open("default.conf", O_RDONLY | O_NONBLOCK);
    int fr = open("response_test.txt", O_WRONLY | O_NONBLOCK | O_CREAT | O_TRUNC, 0777);

	if (f <= 0)
	{
		perror("default.conf");
		return 1;
	}

	HTTPError err;
    HTTPRequest request;
    RequestManager requestmanager(request, err, SysBufferFactory::SYSBUFF_FILE, f);
	ResponseManager responsemanager(request, err, SysBufferFactory::SYSBUFF_FILE, fr);

    while (1)
    {
        requestmanager.process();
        if (requestmanager.gerError().status() != OK || requestmanager.request_done())
		{
			generate_rq_response(responsemanager);
            break ;
		}
    }
}