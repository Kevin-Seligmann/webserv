#include "../inc/VirtualServersManager.hpp"
#include "../inc/Utils.hpp"

VirtualServersManager::VirtualServersManager() {}

VirtualServersManager::~VirtualServersManager() {
	cleanupSockets();
}

void VirtualServersManager::addServer(Servers::Servers::VirtualServerInfo& server) {

	const Listen::Listen::VirtualServerKey& listen = server.getListen();
	VirtualServersMap::iterator it = serversManager.find(listen);

	if (it != serversManager.end()) {
		if (server.getListen().is_default)
			server.setListen(Listen::Listen::VirtualServerKey(listen.host, listen.port, false));
		it->second.push_back(server);
	} 
	else {
		server.setListen(Listen::Listen::VirtualServerKey(listen.host, listen.port, true));
		serversManager[listen] = VirtualServerGroup(1, server);
	}
}

bool VirtualServersManager::loadFromParsedConfig(const ParsedServers& ps) {
	for (size_t i = 0; i < ps.size(); ++i) {
		const ParsedServer& config = ps[i];		
		if (config.listens.empty()) {
			Listen::VirtualServerKey defaultListen;
			Servers::VirtualServerInfo server(config, defaultListen);
			addServer(server);
		} else {
			for (size_t j = 0; j < config.listens.size(); ++j) {
				const Listen::VirtualServerKey& customListen = config.listens[j];
				Servers::VirtualServerInfo server(config, customListen);
				addServer(server);
			}
		}
	}
	OKlogsEntry("SUCCESS: ", "Configuration loaded form config file.");
	return (true);
}

bool VirtualServersManager::initializeSockets(void) {
	VirtualServersMap::iterator it = serversManager.begin();
	for (; it != serversManager.end(); ++it) {
		const Listen::VirtualServerKey& key = it->first;

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == -1) {
			ERRORlogsEntry("ERROR: couldn't create socket. ", strerror(errno));
			cleanupSockets();
			throw std::runtime_error("Failed to create socket");
			return (false); // código muerto, nunca se ejecuta, solo para posible queja de compilador.
		}

		// AQUI TIMEOUT?

		int opt = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
			ERRORlogsEntry("ERROR: ", "couldn't setsockopt SO_REUSEADDR: " + std::string(strerror(errno)));
			cleanupSockets();
			close(fd);
			throw std::runtime_error("Failed to set socket options.");
			return (false); // código muerto, nunca se ejecuta, solo para posible queja de compilador.
		}

		int flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1) {
			ERRORlogsEntry("ERROR: couldn't get socket flags.", strerror(errno));
			cleanupSockets();
			close(fd);
			throw std::runtime_error("Failed to get flags.");
			return (false); // código muerto, nunca se ejecuta, solo para posible queja de compilador.
		}
		
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
			ERRORlogsEntry("ERROR: ", "couldn't set socket flags: " + std::string(strerror(errno)));
			cleanupSockets();
			close(fd);
			throw std::runtime_error("Failed to set flags.");
			return (false); // código muerto, nunca se ejecuta, solo para posible queja de compilador.
			
		}
		serversToSockets[key] = fd;
	}
	OKlogsEntry("SUCCESS: ", "Sockets initialized");
	return (true);
}

bool VirtualServersManager::bindSockets(void) {
	ServersToSocketsMap::iterator it = serversToSockets.begin();
	for (; it != serversToSockets.end(); ++it) {
		const Listen::VirtualServerKey& serverKey = it->first;
		int socketFd = it->second;

		// Prepare address structure for binding
		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(serverKey.port);

		if (serverKey.host == "0.0.0.0" || serverKey.host.empty()) {
			server_addr.sin_addr.s_addr = INADDR_ANY;
		} else {
			struct addrinfo hints, *result;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;

			int ret = getaddrinfo(serverKey.host.c_str(), NULL, &hints, &result);
			if (ret != 0) {
				std::string error_msg = "couldn't complete getaddrinfo for " + serverKey.host + ": " + gai_strerror(ret);
				ERRORlogsEntry("ERROR: ", error_msg);
				cleanupSockets();
				throw std::runtime_error("Invalid host address");
				return (false); // código muerto, nunca se ejecuta, solo para posible queja de compilador.
			}

			struct sockaddr_in* addr_in = (struct sockaddr_in*)result->ai_addr;
			server_addr.sin_addr = addr_in->sin_addr;
			freeaddrinfo(result);
		}

		if (bind(socketFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
			ERRORlogsEntry("ERROR: ", "couldn't bind sockets: " + std::string(strerror(errno)));
			cleanupSockets();
			throw std::runtime_error(strerror(errno));
			return (false); // código muerto, nunca se ejecuta, solo para posible queja de compilador.
		}
	}
	OKlogsEntry("SUCCESS: ", "Sockets binded.");
	return (true);
}

bool VirtualServersManager::socketsListen(void) {
	VirtualServersManager::ServersToSocketsMap::iterator it = serversToSockets.begin();
	for (; it != serversToSockets.end(); ++it) {
		if (listen(it->second, SOMAXCONN) == -1) {
			ERRORlogsEntry("ERROR: ", "couldn't set listen for " + it->first.host + ": " + strerror(errno));
			cleanupSockets();
			throw std::runtime_error(strerror(errno));
			return (false); // código muerto, nunca se ejecuta, solo para posible queja de compilador.
		}
	}
	OKlogsEntry("SUCCESS: ", "Sockets set to listen.");
	return (true);
}

void VirtualServersManager::cleanupSockets() {
	for (std::map<Listen::VirtualServerKey, int>::iterator it = serversToSockets.begin(); 
		 it != serversToSockets.end(); ++it) {
		close(it->second);
	}
	serversToSockets.clear();
}

Servers::VirtualServerInfo* VirtualServersManager::getServerForKey(const Listen::VirtualServerKey& key) {
	VirtualServersMap::iterator it = serversManager.find(key);
	if (it != serversManager.end() && !it->second.empty()) {
		return &(it->second[0]);
	}
	return NULL;
}

