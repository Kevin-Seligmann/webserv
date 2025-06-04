#include "../inc/VirtualServersManager.hpp"
#include "../inc/Parsed.hpp"
#include "../inc/Utils.hpp"
#include "../inc/debug.hpp"
#include <iostream>

int main(int argc, char** argv) {
	try {
		ParsedServers config;
		if (parseProcess(argc, argv, config) != 0) return (1);
		showParse(config);

		VirtualServersManager webServers;
		if (!serversInit(webServers, config)) {
			std::cerr << RED << "Failed to initialize servers." << RESET << std::endl;
			return (1);
		}
		showServers(webServers);

		// =====================================================================
		// FASE 3: MAIN EVENT LOOP (FUTURO)
		// =====================================================================
		// TODO: Implementar event loop con epoll
		// 1. Crear epoll instance
		// 2. Registrar listen sockets en epoll usando punteros a VirtualServerKey
		// 3. Event loop principal para manejar conexiones
		
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
}