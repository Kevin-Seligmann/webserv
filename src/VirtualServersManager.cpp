#include "VirtualServersManager.hpp"
#include <cstring>  // memset
#include <unistd.h>  // close
#include <arpa/inet.h>  // inet_pton

// ================ CONSTRUCTORS & DESTRUCTOR ================

VirtualServersManager::VirtualServersManager(){
}

VirtualServersManager::VirtualServersManager(const ParsedServers& configs){

	std::vector<ParsedServer>::const_iterator it = configs.begin();
	for (;it!= configs.end(); ++it) {
		_server_configs.push_back(ServerConfig(*it));
	}

	for (size_t i = 0; i < _server_configs.size(); ++i) {
		ServerConfig* cfg = &_server_configs[i];
		const ParsedServer& parsed = configs[i];

		std::vector<Listen>::const_iterator l_it = parsed.listens.begin();
		for (; l_it != parsed.listens.end(); ++l_it) {
			const Listen listen = *l_it;
			// Mapeo de listen y vector de configuraciones
			_virtual_hosts[listen].push_back(cfg);

			// Par único listen - socket (será creado)
			if (_listen_sockets.find(listen) == _listen_sockets.end()) {
				_listen_sockets[listen] = -1; // Asignar socket en POLLING
			} 
		}
	}
	Logger::getInstance().info("VirtualServersManager: Loaded " + wss::i_to_dec(_server_configs.size()) + " configs");
    Logger::getInstance().info("VirtualServersManager: Created " + wss::i_to_dec(_listen_sockets.size()) + " listen ports");
}

VirtualServersManager::~VirtualServersManager() {
	// Sockets
	std::map<Listen, int>::iterator sock_it = _listen_sockets.begin();
	for (;sock_it != _listen_sockets.end(); ++sock_it) {
		if (sock_it->second >= 0) {
			close(sock_it->second);
		}
	}

	// Clients
	std::map<int, Client*>::iterator cli_it = _clients.begin();
	for (; cli_it != _clients.end(); ++cli_it) {
		delete cli_it->second;
	}
	_clients.clear();

	Logger::getInstance().info("VirtualServersManager destroyed");
}

// ================ SOCKETS / POLL =================

void VirtualServersManager::setPolling() {
    Logger::getInstance().info("Starting polling configuration (setPolling)");
    
    Logger::getInstance() << "Creating sockets for " << _server_configs.size() << " servers" << std::endl;
    
	for (std::map<Listen, int>::iterator it = _listen_sockets.begin();
		it != _listen_sockets.end(); ++it) {

		it->second = createAndBindSocket(it->first);
		Logger::getInstance() << "Server " << it->first.host << ":" << it->first.port 
							  << " started on socket FD: " << it->second;

		_wspoll.add(it->second, POLLIN);
		Logger::getInstance().info("Socket created and added to poll");
    }
	Logger::getInstance().info("Setup of poll complete");
}	

int VirtualServersManager::createAndBindSocket(const Listen& listen_arg) {
	int socket_fd = socket (AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		throw std::runtime_error("Faile to creat socket");
	}

	int opt = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listen_arg.port);

	if (listen_arg.host == "0.0.0.0" || listen_arg.host.empty()) {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		inet_pton(AF_INET, listen_arg.host.c_str(), &addr.sin_addr);
	}

	if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(socket_fd);
		throw std::runtime_error("Bind failed");
	}

	if (listen(socket_fd, 128) < 0) {
		close (socket_fd);
		throw std::runtime_error("Listen failed");
	}
	return socket_fd;
}

bool VirtualServersManager::isListenSocket(int socket_fd) const {
	for (std::map<Listen, int>::const_iterator it = _listen_sockets.begin();
		it != _listen_sockets.end(); ++it) {
		if (it->second == socket_fd) {
			return true;
		}
	}
	return false;
}

// ================ CLIENT ================

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

void VirtualServersManager::disconnectClient(int client_fd) {
	std::cout << "Disconnecting client FD: " << client_fd << std::endl;
	
	_client_to_listen.erase(client_fd);

	std::map<int, Client*>::iterator it = _clients.find(client_fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
		return ;
	}

	CODE_ERR("The server tried to find a client that does not exists. This is not possible.");
}

// ================ EVENT ================

void VirtualServersManager::handleEvent(const struct Wspoll_event event) {
	int socket_fd = event.fd;
	Logger::getInstance() << "Handling " << socket_fd << std::endl;

	if (event.events & POLLIN && isListenSocket(socket_fd)) {
		handleNewConnection(socket_fd);
	} else {
		try {
			Client* client = searchClient(socket_fd);
			if (!client)
				CODE_ERR("A file descriptor that doesn't belong to any client has been found");

			if (event.events & (POLLRDHUP | POLLHUP | POLLERR))
			{
				if (event.events & POLLRDHUP) // Better to set up a close routine. Client might still waits for a response.
					Logger::getInstance() << "Client " + wss::i_to_dec(socket_fd) + " closed the connection." << std::endl;
				else if (event.events & POLLHUP)
					Logger::getInstance() << "Client " + wss::i_to_dec(socket_fd) + " closed the connection abrutply." << std::endl;
				if (event.events & POLLRDHUP)
					Logger::getInstance() << "Client " + wss::i_to_dec(socket_fd) + " socket error. Closing connection abruptly." << std::endl;

				disconnectClient(socket_fd);
			}
			else 
			{
				client->process(socket_fd, event.events);
			}
				
		} catch (const std::runtime_error& e) {
			std::cerr << "Exception processing client data: " << e.what() << std::endl;
			disconnectClient(socket_fd);
		}
	}
}

void VirtualServersManager::handleNewConnection(int listen_fd) {
	Listen* listen = NULL;
	for (std::map<Listen, int>::iterator it = _listen_sockets.begin();
		it != _listen_sockets.end(); ++it) {
			if (it->second == listen_fd) {
				listen = const_cast<Listen*>(&it->first);
				break;
			}
	}
	
	if (!listen) {
		return;
	}

	struct sockaddr_in client_addr;
	std::memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		return;
	}

	_client_to_listen[client_fd] = *listen;
	_clients[client_fd] = new Client(*this, client_fd);
}

// ================ POLL FD MANAGEMENT ================

void VirtualServersManager::hookFileDescriptor(ActiveFileDescriptor const & actf)
{
	_wspoll.add(actf.fd, actf.mode);
}

void VirtualServersManager::unhookFileDescriptor(ActiveFileDescriptor const & actf)
{
	_wspoll.del(actf.fd);
}

void VirtualServersManager::swapFileDescriptor(ActiveFileDescriptor const & oldfd, ActiveFileDescriptor const & newfd)
{
	if (oldfd == newfd)
		return;
	if (oldfd.fd == newfd.fd)
		_wspoll.mod(newfd.fd, newfd.mode);
	else 
	{
		unhookFileDescriptor(oldfd);
		hookFileDescriptor(newfd);
	}
}

// ================ REQUEST ROUTER ================

ServerConfig* VirtualServersManager::findServerConfigForRequest(const HTTPRequest& request, int client_fd) {
	std::map<int, Listen>::iterator it = _client_to_listen.find(client_fd);
	if (it == _client_to_listen.end())
		CODE_ERR("Impossible error trying to find server for client " + wss::i_to_dec(client_fd));

	std::map<Listen, std::vector<ServerConfig*> >::iterator vh_it = _virtual_hosts.find(it->second);
	if (vh_it == _virtual_hosts.end())
		CODE_ERR("Impossible error trying to find server for client " + wss::i_to_dec(client_fd));

	std::string hostname = request.get_host();
	for (size_t i = 0; i < vh_it->second.size(); ++i){
		if (vh_it->second[i]->matchesServerName(hostname)) {
			return vh_it->second[i];
		}
	}

	return vh_it->second[0]; // First one is default
}

// ================ MAIN LOOP ================

void VirtualServersManager::run() {
	Logger::getInstance().info("=========== Starting WEBSERVER ===========");

	try {
		setPolling();
	} catch (const std::exception& e) {
		Logger::getInstance().error("Setup falied: " + std::string(e.what()));
		return;
	}

	Logger::getInstance().info("=========== WEBSERVER Started. Listening to new requests ===========");

	while (1) {

		int incoming = _wspoll.wait();
	
		Logger::getInstance() << "Starting to process: " << incoming << " events" << std::endl;

		if (incoming == 0) {
			checkTimeouts();
			continue;
		}

		if (incoming < 0) {
			Logger::getInstance().error("Wspoll failed: " + std::string(strerror(errno)));
			break;
		}


		for (int i = 0; i < _wspoll.size(); ++i) {
			if (_wspoll[i].events)
			{
				try {
					handleEvent(_wspoll[i]);
					} 
				catch (const std::exception& e) {
					Logger::getInstance().error("Critial error handling event: " + std::string(e.what()) + " The server must close. ");
					throw std::exception();
				}
			}

		}
	}

	Logger::getInstance().info("=========== Closing EVENT LOOP ===========");
}

void VirtualServersManager::checkTimeouts() {
	time_t now = time(NULL);
	std::vector<int> to_disconnect;

	std::map<int, Client*>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it) {
		if (now - it->second->getLastActivity() > Client::TIMEOUT_SECONDS) {
			to_disconnect.push_back(it->first);
		}
	}

	for (size_t i = 0; i < to_disconnect.size(); ++i) {
		Logger::getInstance().warning("Client " + wss::i_to_dec(to_disconnect[i])
									  + " timeout");
		disconnectClient(to_disconnect[i]);
	}
}
