#include "../inc/VirtualServersManager.hpp"
#include "../inc/Utils.hpp"

VirtualServersManager::VirtualServersManager() {}

VirtualServersManager::~VirtualServersManager() {
	cleanupSockets();
}

void VirtualServersManager::addServer(VirtualServerInfo& server) {

	const VirtualServerKey& listen = server.getListen();
	VirtualServersMap::iterator it = serversManager.find(listen);

	if (it != serversManager.end()) {
		if (server.getListen().is_default)
			server.setListen(VirtualServerKey(listen.host, listen.port, false));
		it->second.push_back(server);
	} 
	else {
		server.setListen(VirtualServerKey(listen.host, listen.port, true));
		serversManager[listen] = VirtualServerGroup(1, server);
	}
}

bool VirtualServersManager::loadFromParsedConfig(const ParsedServers& ps) {
	for (size_t i = 0; i < ps.size(); ++i) {
		const ParsedServer& config = ps[i];		
		if (config.listens.empty()) {
			VirtualServerKey defaultListen;
			VirtualServerInfo server(config, defaultListen);
			addServer(server);
		} else {
			for (size_t j = 0; j < config.listens.size(); ++j) {
				const VirtualServerKey& customListen = config.listens[j];
				VirtualServerInfo server(config, customListen);
				addServer(server);
			}
		}
	}
	return (true);
}

bool VirtualServersManager::initializeSockets(void) {
	VirtualServersMap::iterator it = serversManager.begin();
	for (; it != serversManager.end(); ++it) {
		const VirtualServerKey& key = it->first;

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == -1) {
			ERRORlogsEntry("ERROR: couldn't create socket. ", strerror(errno));
			cleanupSockets();
			throw std::runtime_error("Failed to create socket");
			return (false);
		}

		int opt = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) == -1)) {
			ERRORlogsEntry("ERROR: couldn't setsockopt SO_REUSEADDR.", strerror(errno));
			cleanupSockets();
			close(fd);
			throw std::runtime_error("Failed to set socket options.");
			return (false);
		}

		int flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1) {
			ERRORlogsEntry("ERROR: couldn't get socket flags.", strerror(errno));
			cleanupSockets();
			close(fd);
			throw std::runtime_error("Failed to get flags.");
			return (false);
		}
		
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
			ERRORlogsEntry("ERROR: couldn't set socket flags.", strerror(errno));
			cleanupSockets();
			close(fd);
			throw std::runtime_error("Failed to set flags.");
			return (false);
		}

		serversToSockets[key] = fd;

	}
	return (true);
}

bool VirtualServersManager::bindSockets(void) {
	ServersToSocketsMap::iterator it = serversToSockets.begin();
	for (; it != serversToSockets.end(); ++it) {
		VirtualServerKey serverKey = it->first;
		int socket = it->second;

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(serverKey.port);
		if (serverKey.host == "0.0.0.0" || serverKey.host.empty()) {
			server_addr.sin_addr.s_addr = INADDR_ANY;
		} else {
			server_addr.sin_addr.s_addr = inet_addr(serverKey.host.c_str());
		}


	}

}

/* 
bool VirtualServersManager::initAllSockets() {
	for (VirtualServersMap::iterator it = serversManager.begin(); it != serversManager.end(); ++it) {
		const VirtualServerKey& key = it->first;
		if (!createListenSocket(key)) {
			std::cerr << "Error: Failed to create socket for " << key.host << ":" << key.port << std::endl;
			cleanupSockets();
			return false;
		}
	}
	return true;
}

bool VirtualServersManager::createListenSocket(const VirtualServerKey& key) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		return false;
	}

	configureSocket(fd);
	
	if (!bindAndListen(fd, key)) {
		close(fd);
		return false;
	}

	listenSockets[key] = fd;
	return true;
}

void VirtualServersManager::configureSocket(int fd) {
	int opt = 1;
	
	// SO_REUSEADDR - Allow reuse of local addresses
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << "Warning: setsockopt SO_REUSEADDR failed: " << strerror(errno) << std::endl;
	}

	// SO_REUSEPORT - Allow multiple servers on same port (Linux)
	#ifdef SO_REUSEPORT
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
		std::cerr << "Warning: setsockopt SO_REUSEPORT failed: " << strerror(errno) << std::endl;
	}
	#endif

	// Set non-blocking
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Warning: fcntl F_GETFL failed: " << strerror(errno) << std::endl;
		return;
	}
	
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Warning: fcntl F_SETFL failed: " << strerror(errno) << std::endl;
	}
}

bool VirtualServersManager::bindAndListen(int fd, const VirtualServerKey& key) {
	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(key.port);

	// Convert host string to in_addr
	if (key.host == "0.0.0.0" || key.host.empty()) {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		if (inet_pton(AF_INET, key.host.c_str(), &addr.sin_addr) != 1) {
			std::cerr << "Error: Invalid IP address: " << key.host << std::endl;
			return false;
		}
	}

	// Bind
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		std::cerr << "Error binding to " << key.host << ":" << key.port 
				  << " - " << strerror(errno) << std::endl;
		return false;
	}

	// Listen
	if (listen(fd, SOMAXCONN) == -1) {
		std::cerr << "Error listening on " << key.host << ":" << key.port 
				  << " - " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}

*/

void VirtualServersManager::cleanupSockets() {
	for (std::map<VirtualServerKey, int>::iterator it = serverToSocket.begin(); 
		 it != serverToSocket.end(); ++it) {
		close(it->second);
	}
	serverToSocket.clear();
}

