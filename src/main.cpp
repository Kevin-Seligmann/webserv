#include "../inc/VirtualServersManager.hpp"
#include "../inc/Parsed.hpp"
#include "../inc/Utils.hpp"
#include "../inc/debug.hpp"
#include <iostream>

int main(int argc, char** argv) {
	ParsedServers config;
	VirtualServersManager webServers;
	try {
		parseProcess(argc, argv, config);
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
	webServers.cleanupSockets();
	return (0);
}