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
