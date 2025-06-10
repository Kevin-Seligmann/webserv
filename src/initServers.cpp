#include "../inc/VirtualServersManager.hpp"
#include "../inc/Utils.hpp"
#include <iostream>
#include <sstream>

bool serversInit(VirtualServersManager& webServers, const ParsedServers& ps) {
	
	webServers.loadFromParsedConfig(ps);
	
	webServers.initializeSockets();
	
	webServers.bindSockets();

	webServers.socketsListen();

	return true;
}


/*
	// =====================================================================
	// INFORMACION DE DEBUG
	// =====================================================================
	
	std::cout << YELLOW << "Sockets creados:" << RESET << std::endl;
	for (std::map<VirtualServerKey, int>::const_iterator it = webServers.serverToSocket.begin(); 
		 it != webServers.serverToSocket.end(); ++it) {
		const VirtualServerKey& key = it->first;
		int fd = it->second;
		size_t serverCount = webServers.serversManager[key].size();
		
		std::cout << "  - " << key.host << ":" << key.port 
				  << " (fd=" << fd << ")" 
				  << " [" << serverCount << " servidores virtuales]" << std::endl;
	}
	
	std::cout << GREEN << "SUCCESS: Servidor HTTP multiplex inicializado." << RESET << std::endl;
	return true;
}*/