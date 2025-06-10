#include "../inc/VirtualServersManager.hpp"
#include "../inc/Utils.hpp"

bool serversInit(VirtualServersManager& webServers, const ParsedServers& ps) {
	
	webServers.loadFromParsedConfig(ps);
	
	webServers.initializeSockets();
	
	webServers.bindSockets();

	webServers.socketsListen();

	return true;
}

bool serversStart(VirtualServersManager& webServers) {
	
	return true;
}