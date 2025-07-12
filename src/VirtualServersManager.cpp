#include "VirtualServersManager.hpp"

// ================ STATIC MEMBERS ================

std::map<int, VirtualServersManager::ClientState*> VirtualServersManager::ClientState::client_states;


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

VirtualServersManager::ClientState* VirtualServersManager::ClientState::getOrCreateClientState(int client_fd) {
	std::map<int, ClientState*>::iterator it = client_states.find(client_fd);
	if (it != client_states.end()) {
		return it->second;
	}

	ClientState* state = new ClientState(client_fd);
	client_states[client_fd] = state;
	return state;
}

void VirtualServersManager::ClientState::cleanupClientState(int client_fd) {
	std::map<int, ClientState*>::iterator it = client_states.find(client_fd);
	if (it != client_states.end()) {
		delete it->second;
		client_states.erase(it);
	}
}

// ================ PRIVATE METHODS ================

void VirtualServersManager::setupEpoll() {
	std::cout << "Starting epoll configuration" << std::endl;
	
	_epoll_fd = epoll_create(1);
	if (_epoll_fd < 0) {
		throw std::runtime_error("Falied to create epoll fd");
	}
	std::cout << "Epoll created with fd: " << _epoll_fd << std::endl;

	std::cout << "Creating sockets for " << _servers.size() << " servers" << std::endl;
	for (size_t i = 0; i < _servers.size(); ++i) {
		try {
			_servers[i].start(); // socket() + bind() + listen()
			std::cout << "Server " << i << " started on FD: " << _servers[i].getSocketFD() << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "Falied to start server " << i << ": " << e.what() << std::endl;
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
	std::cout << "Epoll configuration ready! Monotoring " << _servers.size() << " servers" << std::endl;
	// llevar la cuenta de los que fallan para restarlo de _servers.size() ??
}

bool VirtualServersManager::isServerFD(int fd) const {
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getSocketFD() == fd)
			return true; // es server
	}
	return false; // es cliente
}

int VirtualServersManager::findServerIndex(int fd) const {
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getSocketFD() == fd)
			return static_cast<int>(i);
	}
	return -1;
}


void VirtualServersManager::disconnectClient(int client_fd) {
	std::cout << "Disconnecting client FD: " << client_fd << std::endl;
	
	ClientState::cleanupClientState(client_fd);
	
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	
	close(client_fd);
	
	std::vector<int>::iterator it = std::find(_client_fds.begin(), _client_fds.end(), client_fd);
	if (it != _client_fds.end())
		_client_fds.erase(it);
	
	std::cout << "Client FD: " << client_fd << " cleaned up" << std::endl;
}

void VirtualServersManager::handleEvent(const struct epoll_event& event) {
	int fd = event.data.fd;

	if (event.events & EPOLLIN) {
		if (isServerFD(fd)) {
			int server_index = findServerIndex(fd);
			if (server_index >= 0) {
				handleNewConnection(server_index);
			}
		}
		else {
			handleClientData(fd);
		}
	}
	if (event.events & (EPOLLHUP | EPOLLERR)) {
		if (!isServerFD(fd)) {
			disconnectClient(fd);
		}
	}
}

void VirtualServersManager::processCompleteRequest(int client_fd, HTTPRequest& request) {
	std::cout << "\tComplete request:" << std::endl;
	std::cout << "\t\tMethod: " << request.method << std::endl;
	std::cout << "\t\tHost: " << request.get_host() << std::endl;
	std::cout << "\t\tPath: " << request.get_path() << std::endl;
	std::cout << "\t\tPort: " << request.get_port() << std::endl;

	Server* target_server = findServerForRequest(request);
	if (!target_server) {
		sendErrorResponse(client_fd, 404, "Server not found");
		return;
	}

	Location* location = target_server->getConfig().findLocation(request.get_path());
	if (!location) {
		sendErrorResponse(client_fd, 404, "Location not found");
		return;
	}

	if (!isMethodAllowed(location, request.method)) {
		sendErrorResponse(client_fd, 405, "Method not allowed");
		return;
	}

	if (isCgiRequest(location, request.get_path())) {
		processCgiRequest(client_fd, request, location);
	}
	else {
		processStaticRequest(client_fd, request, location);
	}
}

void VirtualServersManager::handleClientData(int client_fd) {
	ClientState* client = ClientState::getOrCreateClientState(client_fd);

	try {
		client->request_manager->process();

		if (client->hasError()) {
			std::cerr << "Error parsing request: " << client->error.to_string() << std::endl;
			sendErrorResponse(client_fd, client->error);
			disconnectClient(client_fd);
			return;
		}

		if (client->isRequestComplete()) {
			std::cout << "Request complete for FD: " << client_fd << std::endl;
			processCompleteRequest(client_fd, client->request);
//			client->request_manager->new_request();
// activarlo cuando este ok
			disconnectClient(client_fd);
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception processing client data: " << e.what() << std::endl;
		disconnectClient(client_fd);
	}
}

void VirtualServersManager::handleNewConnection(int server_index) {
	std::cout << "New connection at server " << server_index << std::endl;

	int server_fd = _servers[server_index].getSocketFD();

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

	if (client_fd < 0) {
		std::cerr << "Failed to accept connection on server " << server_index << std::endl;
	}
	
	std::cout << "Client connected @ FD: " << client_fd << std::endl;

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
		close(client_fd);
		return;
	}

	_client_fds.push_back(client_fd);
}

void VirtualServersManager::temp_sendSimpleResponse(int client_fd) {
	std::string body = "<html><body><h1>Hello from Webserv!</h1></body></html>";
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "\r\n"
			 << body;
	
	std::string response_str = response.str();
	send(client_fd, response_str.c_str(), response_str.length(), 0);
}

// ================ HELPER METHODS ================

Server* VirtualServersManager::findServerForRequest(const HTTPRequest& request) {
	// Implement match based on Host header and port
	(void)request;
	if (!_servers.empty()) {
		return &_servers[0];
	}
	return NULL;
}

bool VirtualServersManager::isMethodAllowed(Location* location, HTTPMethod method) {
	// Implement check based on location._methods
	(void)location;
	(void)method;
	return true;
}

void VirtualServersManager::sendErrorResponse(int client_fd, int status_code, const std::string& message) {
	std::ostringstream response;
	response << "HTTP/1.1 " << status_code << " " << message << "\r\n"
			 << "Content-Type: text/plain\r\n"
			 << "Content-Length: " << message.length() << "\r\n"
			 << "\r\n"
			 << message;
	
	std::string response_str = response.str();
	send(client_fd, response_str.c_str(), response_str.length(), 0);
}

void VirtualServersManager::sendErrorResponse(int client_fd, const HTTPError& error) {
	int status_code = static_cast<int>(error.status());
	sendErrorResponse(client_fd, status_code, error.msg());
}

// ================ PUBLIC ================

void VirtualServersManager::run() {

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

		if (incoming == 0) {
			continue;
		}

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

// ================ REQUEST PROCESSING METHODS ================

bool VirtualServersManager::isCgiRequest(Location* location, const std::string& path) {
	// Implement CGI detection based on location configuration
	(void)location;
	(void)path;
	return path.find(".cgi") != std::string::npos; // true si uri de la request termina en cgi
}

void VirtualServersManager::processCgiRequest(int client_fd, HTTPRequest& request, Location* location) {
	// Implement CGI request processing
	(void)request;
	(void)location;
	
	std::string response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 27\r\n"
		"\r\n"
		"CGI processing placeholder";
	
	send(client_fd, response.c_str(), response.length(), 0);
}

void VirtualServersManager::processStaticRequest(int client_fd, HTTPRequest& request, Location* location) {
	// Implement static HTML
	(void)request;
	(void)location;
	
	temp_sendSimpleResponse(client_fd);
}