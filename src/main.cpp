
// SERVER TEST
#include "VirtualServersManager.hpp"
#include "Parsed.hpp"
#include "Utils.hpp"
#include "debug.hpp"
#include <iostream>

int main(int argc, char** argv) {
	ParsedServers config;
	VirtualServersManager webServers;
	try {
		if (parseProcess(argc, argv, config) != 0) return (1);
		
		serversInit(webServers, config);
			
		showParse(config);
		showServers(webServers);
		showSockets(webServers);
		
		std::cout << GREEN << "Server ready. Event loop will be implemented next." << RESET << std::endl;
		OKlogsEntry("SUCCESS: ", "Servers ready to operate.");
		event_loop(webServers);

	} catch (const std::runtime_error& e) {
		std::cerr << RED << "RUNTIME ERROR: " << RESET << e.what() << std::endl;
		return (1);
	} catch (const std::exception& e) {
		std::cerr << RED << "STD_EXCEPTION ERROR: " << RESET << e.what() << std::endl;
		return (1);
	} catch (...) {
		std::cerr << RED << "WIRED ERROR: " << RESET << "Unknown exception occurred" << std::endl;
		return (1);
	}
}


// REQUEST TEST
/*
	Use this to test Responses or CGI. Modify file 'request_test.txt'
	If the requests hangs is because it's not finished (For example, forgetting a LF after the headers)
*/
// #include <fcntl.h>
// #include <errno.h>
// #include "RequestManager.hpp"
// #include "HTTPRequest.hpp"

// int main(int argc, char ** argv)
// {
//     int f = open("request_test.txt", O_RDONLY);
// 	if (f <= 0)
// 	{
// 		perror("request_test.txt");
// 		return 1;
// 	}
//     HTTPRequest request;
//     RequestManager manager(request, SysBufferFactory::SYSBUFF_FILE, f);
//     while (1)
//     {
//         manager.process();
//         if  (manager.gerError().status() != OK)
// 		{
// 			std::cout << "Fail" << std::endl;
// 			// Do anything with failed request
//             break ;
// 		}
//         if (manager.request_done())
// 		{
// 			std::cout << request;
// 			// Do anything with done request
// 			break ;
// 		}
//     }}
