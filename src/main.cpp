#include "../inc/VirtualServersManager.hpp"
#include "../inc/Parsed.hpp"
#include "../inc/Utils.hpp"
#include "../inc/debug.hpp"
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

		return (0);
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
	webServers.cleanupSockets();
}