#include "../inc/VirtualServersManager.hpp"
#include "../inc/Parsed.hpp"
#include "../inc/Utils.hpp"
#include "../inc/debug.hpp"
#include <iostream>

int main(int argc, char** argv) {
	try {
		ParsedServers config;
		if (parseProcess(argc, argv, config) != 0) return (1);
		
		// el control de error de este bloque se hace mediante try catch por lo que es innecesario(?) el if
		// revisar desded aqui
		VirtualServersManager webServers;
		if (!serversInit(webServers, config)) {
			std::cerr << RED << "Failed to initialize servers." << RESET << std::endl;
			return (1);
		}
		// hasta aqui
			
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
//	webServers.cleanupSockets();
}