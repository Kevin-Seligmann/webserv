#include "VirtualServersManager.hpp"

// ================ CONSTRUCTORS & DESTRUCTOR ================

VirtualServersManager::VirtualServersManager()
	: _epoll_fd(-1) {
	_events.resize(64);
}

VirtualServersManager::VirtualServersManager(const ParsedServers& configs) 
	: _epoll_fd(-1) {
	_events.resize(64);

	std::vector<ParsedServer>::const_iterator it = configs.begin();
	for (; it != configs.end(); ++it) {
		try {
			_servers.push_back(Server(*it));
		} catch (const std::exception& e) {
			std::cerr << "Error creating server: " << e.what() << std::endl;
		}
	}
}

VirtualServersManager::~VirtualServersManager() {
	if (_epoll_fd >= 0) {
		close(_epoll_fd);
	}
}


// ================ CLIENT STATE MANAGEMENT ================
Client* VirtualServersManager::searchClient(int client_fd) {
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	if (it != _clients.end()) {
		return it->second;
	}

	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it ++)
		if (it->second->ownsFd(client_fd))
			return it->second;
	return NULL;
}


void VirtualServersManager::cleanupClientState(int client_fd) {
	std::map<int, Client*>::iterator it = _clients.find(client_fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
		return ;
	}
	CODE_ERR("The server tried to find a client that does not exists. This is not possible.");
}

// ================ AUX FUNCTIONS ================

bool VirtualServersManager::isServerFD(int fd) const {
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getSocketFD() == fd)
			return true; // es server
	}
	return false; // es cliente
}


// ================ EVENT HANDLER ================

void VirtualServersManager::handleEvent(const struct epoll_event& event) {
	int fd = event.data.fd;

	if (event.events & EPOLLIN && isServerFD(fd)) {
		int server_index = findServerIndex(fd);
		if (server_index >= 0) {
			handleNewConnection(server_index);
		}
	}
	else {
		try {
			Client* client = searchClient(fd);
			if (!client)
				CODE_ERR("A file descriptor that doesn't belong to any client has been found");

			if (event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				if (event.events & EPOLLRDHUP) // Better to set up a close routine. Client might still waits for a response.
					Logger::getInstance() << "Client " + wss::i_to_dec(fd) + " closed the connection." << std::endl;
				else if (event.events & EPOLLHUP)
					Logger::getInstance() << "Client " + wss::i_to_dec(fd) + " closed the connection abrutply." << std::endl;
				if (event.events & EPOLLRDHUP)
					Logger::getInstance() << "Client " + wss::i_to_dec(fd) + " socket error. Closing connection abruptly." << std::endl;

				disconnectClient(fd);
			}
			else 
			{
				client->process(fd, event.events);
			}
				
		} catch (const std::exception& e) {
			std::cerr << "Exception processing client data: " << e.what() << std::endl;
			disconnectClient(fd);
		}
	}
	// if (event.events & (EPOLLHUP | EPOLLERR)) {
	// 	if (!isServerFD(fd)) {
	// 		disconnectClient(fd);
	// 	}
	// }
}


// ================ NEW CONNECTION HANDLER ================

void VirtualServersManager::handleNewConnection(int server_index) {
	std::cout << "New connection at server " << server_index << std::endl;

	int server_fd = _servers[server_index].getSocketFD();

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

	if (client_fd < 0) {
		std::cerr << "Failed to accept connection on server " << server_index << std::endl;
		return ;
	}
	
	std::cout << "Client connected @ FD: " << client_fd << std::endl;

	// struct epoll_event event;
	// event.events = EPOLLIN;
	// event.data.fd = client_fd;

	// if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
	// 	close(client_fd);
	// 	return;
	// }

	_client_fds.push_back(client_fd);
	_clients[client_fd] = new Client(*this, _servers, client_fd);
}


// ================ DISCONNECT CLIENT ================

void VirtualServersManager::disconnectClient(int client_fd) {
	std::cout << "Disconnecting client FD: " << client_fd << std::endl;
	
	cleanupClientState(client_fd);
		
	std::vector<int>::iterator it = std::find(_client_fds.begin(), _client_fds.end(), client_fd);
	if (it != _client_fds.end())
		_client_fds.erase(it);
	
	std::cout << "Client FD: " << client_fd << " cleaned up" << std::endl;
}

// ================ PUBLIC ================

void VirtualServersManager::run() {
	// NOTE: En el processCompleteRequest mirar si al salir de location hay error 404 
	// y si hay directory listing habilitado para llamar a un recurso que genere el 
	// listing si es necesario (en lugar de devolver 404 directamente)

	std::cout << std::string(10, '=') << " Starting WEBSERVER " << std::string(10, '=') << std::endl;

	try {
		setupEpoll();
	} catch (const std::exception& e) {
		std::cerr << "Setup falied: " << e.what() << std::endl;
		return;
	}

	std::cout << std::string(10, '=') << " Starting EVENT LOOP " << std::string(10, '=') << std::endl;

	while (1) {

		int incoming = epoll_wait(_epoll_fd, _events.data(), _events.size(), -1);
		
		if (incoming < 0) {
			std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
			break;
		}

/*		if (incoming == 0) {
			continue;
		} // nunca habrá timeout // deberia haber timeout?? */

		std::cout << "Starting to process: " << incoming << " events" << std::endl;

		for (int i = 0; i < incoming; ++i) {
			try {
				handleEvent(_events[i]);
			} catch (const std::exception& e) {
				std::cerr << "Error handling event: " << e.what() << std::endl;
			}
		}
	}
	std::cout << std::string(10, '=') << " Closing EVENT LOOP " << std::string(10, '=') << std::endl;
}


// ================ FD MANAGEMENT ================
void VirtualServersManager::hookFileDescriptor(ActiveFileDescriptor const & actf)
{
    struct epoll_event event;

	event.events = actf.mode;
	event.data.fd = actf.fd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, actf.fd, &event) < 0)
	{
		throw std::runtime_error("Epoll hook failed: " + std::string(strerror(errno)));
	}
	Logger::getInstance() << "Hooked " + wss::i_to_dec(actf.fd) + ", " + wss::i_to_dec(actf.mode) << std::endl;
}

void VirtualServersManager::unhookFileDescriptor(ActiveFileDescriptor const & actf)
{
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, actf.fd, NULL) < 0)
	{
		throw std::runtime_error("Epoll unhook failed: " + std::string(strerror(errno)));
	}
	Logger::getInstance() << "Unhooked " + wss::i_to_dec(actf.fd) + ", " + wss::i_to_dec(actf.mode) << std::endl;
}

void VirtualServersManager::swapFileDescriptor(ActiveFileDescriptor const & oldfd, ActiveFileDescriptor const & newfd)
{
	if (oldfd == newfd)
		return;
	Logger::getInstance() << "Swap " + wss::i_to_dec(oldfd.fd) + ", " + wss::i_to_dec(oldfd.mode) << 
	+ ". For " + wss::i_to_dec(newfd.fd) + ", " + wss::i_to_dec(newfd.mode) << std::endl;
	if (oldfd.fd == newfd.fd)
	{
		struct epoll_event event;

		event.events = newfd.mode;
		event.data.fd = newfd.fd;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, newfd.fd, &event) < 0)
		{
			throw std::runtime_error("Epoll change failed: " + std::string(strerror(errno)));
		}
	}
	else 
	{
		unhookFileDescriptor(oldfd);
		hookFileDescriptor(newfd);
	}
}

// Other
int VirtualServersManager::findServerIndex(int fd) const {
    // Busca el índice del server con el fd dado, o -1 si no existe
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (_servers[i].getSocketFD() == fd)
            return static_cast<int>(i);
    }
    return -1;
}

void VirtualServersManager::setupEpoll() {
    std::cout << "Starting epoll configuration" << std::endl;
    _epoll_fd = epoll_create(1);
    if (_epoll_fd < 0) {
        throw std::runtime_error("Failed to create epoll fd");
    }
    std::cout << "Epoll created with fd: " << _epoll_fd << std::endl;
    std::cout << "Creating sockets for " << _servers.size() << " servers" << std::endl;
    for (size_t i = 0; i < _servers.size(); ++i) {
        try {
            _servers[i].start();
            std::cout << "Server " << i << " started on FD: " << _servers[i].getSocketFD() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to start server " << i << ": " << e.what() << std::endl;
            throw;
        }
    }
    for (size_t i = 0; i < _servers.size(); ++i) {
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = _servers[i].getSocketFD();
        int server_fd = _servers[i].getSocketFD();
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
            std::cerr << "Failed to add server " << i << " with FD: " << server_fd << " to epoll" << std::endl;
            throw std::runtime_error("Failure at epoll_ctl ADD");
        }
        std::cout << "Added server " << i << " with FD: " << server_fd << " to epoll" << std::endl;
    }
    std::cout << "Epoll configuration ready! Monitoring " << _servers.size() << " servers" << std::endl;
}	